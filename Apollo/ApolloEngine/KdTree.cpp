#include "KdTree.h"
#include <assert.h>
#include <algorithm>

using namespace std;

namespace Apollo {

/* defined constants used for constructing the tree */
#define KD_MAX_DEPTH 30  //try 8 + 1.3log(n)
#define KD_MIN_LEAF 2
#define KD_ISECT_COST 5
#define KD_TRAV_COST 1
#define KD_EMPTY_BONUS .8f

#define KD_START_SIZE 1024

/**
 * the layout of the node:
 *	|-|----------------------------|--|
 *	 31         30-2               1-0
 *
 *	 31: 1 is leaf; 0 is internal
 *	 2-30: offset for child
 *	 0-1: splitting dimension
 */
struct KdNode {
    UINT32 d;
    union {
        UINT32 num_children;
	    FLOAT split;
    };
};

enum KdDim { DIM_X, DIM_Y, DIM_Z, DIM_INVALID};

struct KdSplitPlane {	
	enum SPLIT_TYPE {EXIT, PLANE, ENTER};

	KdDim dim;
	FLOAT split;
    UINT32 primitive;
	SPLIT_TYPE type;

	bool operator< (const KdSplitPlane& p) const;
	bool operator== (const KdSplitPlane& p) const;
	void print();
};

struct KdBuildNode {
    bool isLeaf;
    UINT32 num_children;
    UINT32* children;
    KdDim dimension;
    FLOAT split;
    UINT32 offset;
};

vector<KdSplitPlane> all_splits[3];
vector<char> split_buffer;
vector<KdBuildNode> build_tree;

#define KD_ISLEAF(n)	((n)->d & (UINT32)(1 << 31))
#define KD_DIM(n)	((n)->d & 0x3)
#define KD_OFFSET(n)	((n)->d & (0x7FFFFFFC))

#define KD_CHILD(n, d) ((KdNode*)((char*)(n) + KD_OFFSET(n) + ((d) ? 8 : 0)))

#define KD_INIT_LEAF(node, n_children, offset)	\
    node.num_children = n_children; \
    node.d = offset;\
    node.d |= (UINT32)(1 << 31)

#define KD_INIT_INTERNAL(node, dim, offset, s) \
    node.split = s;\
    node.d = offset;\
    node.d |= dim

KdTree::KdTree() {
    m_root = nullptr;
    for (UINT32 i = 0; i < 3; ++i) {
        m_bounds.min[i] = INFINITY;
        m_bounds.max[i] = -INFINITY;
    }
}

KdTree::~KdTree() {
    if (m_root) free(m_root);
}

void KdTree::SetGeometry(std::vector<Model*>& models) {
	m_models.clear();
    m_primitives.clear();
	for (UINT32 i = 0; i < models.size(); i++) {
		AddGeometry(models[i]);
	}
}

bool KdTree::Rebuild() {
    if (!m_dirty) return true;
    for (UINT32 m = 0; m < m_models.size(); m++) {
		m_models[m]->GetPrimitives(m_primitives);
	}
    m_size = 0;
    if (m_root) free(m_root);
    m_root = nullptr;

    build_tree.resize(KD_START_SIZE);

    // Find bounding box
    AABox primitiveBox;
    for (UINT32 i = 0; i < m_primitives.size(); ++i) {
        m_primitives[i]->GetAABoundingBox(primitiveBox);
        m_bounds.Add(primitiveBox);
    }

    // Allocate split plane memory
    for (UINT32 i = 0; i < 3; ++i) {
        all_splits[i].resize(2 * m_primitives.size());
    }
    split_buffer.resize(m_primitives.size());
    m_primitivesIndex.resize(m_primitives.size());
    for (UINT32 i = 0; i < m_primitives.size(); ++i) {
        m_primitivesIndex[i] = i;
    }

    m_nextNode = 2;
    BuildTree(m_bounds, 0, 1, m_primitivesIndex);

    return true;
}

UINT32 KdTree::ComputeSplitPlanes(KdSplitPlane* splits, const vector<UINT32>& primitives, KdDim dim) {
    UINT32 n = 0;
    AABox box;

    for (UINT32 i = 0; i < primitives.size(); ++i) {
        const Primitive* primitive = m_primitives[primitives[i]];
        primitive->GetAABoundingBox(box);
	    // primitive is inside the plane
        if (EQ(box.min[dim], box.max[dim])) {
	        splits[n].dim = dim;
            splits[n].split = (FLOAT)box.min[dim];
	        splits[n].primitive = primitives[i];
	        splits[n].type = KdSplitPlane :: PLANE;
	        n++;
	    } else {
	        splits[n].dim = dim;
            splits[n].split = (FLOAT)box.min[dim];
	        splits[n].primitive = primitives[i];
	        splits[n].type = KdSplitPlane :: ENTER;
	        n++;

	        splits[n].dim = dim;
	        splits[n].split = (FLOAT)box.max[dim];
	        splits[n].type = KdSplitPlane :: EXIT;
	        splits[n].primitive = primitives[i];
	        n++;
	    }
    }		

    if (!n)	return 0;

    sort(splits, splits + n);
    return n;
}

// SAH cost
FLOAT ComputeCost(const AABox bb, FLOAT split, KdDim dim, UINT l, UINT r, UINT p, FLOAT inv_tsa) {
    AABox lBB = bb;
    AABox rBB = bb;
    lBB.max[dim] = split;
    rBB.min[dim] = split;
    FLOAT lsa = lBB.SurfaceArea();
    FLOAT rsa = rBB.SurfaceArea();
    float cost = ((l+p)*lsa + (p+r)*rsa) * inv_tsa * KD_ISECT_COST + KD_TRAV_COST;	
    if (l+p == 0 || r+p == 0) cost *= KD_EMPTY_BONUS;
    return cost;
}
void PartitionTriangles(const KdSplitPlane* splits, UINT32 n, const vector<UINT32> primitives,
    vector<UINT32>& left, vector<UINT32>& right, float split) {

    const char SPLIT_BOTH = 0;
    const char SPLIT_LEFT = 1;
    const char SPLIT_RIGHT = 2;

    memset(&split_buffer[0], SPLIT_BOTH, sizeof(char) * split_buffer.size());

    for (UINT32 i = 0; i < n; ++i) {
	    if (splits[i].type == KdSplitPlane::EXIT && splits[i].split <= split) {
            split_buffer[splits[i].primitive] = SPLIT_LEFT;
	    }
	    else if (splits[i].type == KdSplitPlane::ENTER && splits[i].split >= split) {
            split_buffer[splits[i].primitive] = SPLIT_RIGHT;
	    }
	    else if (splits[i].type == KdSplitPlane :: PLANE) {
	        if (splits[i].split < split) {
                split_buffer[splits[i].primitive] = SPLIT_LEFT;
            } else if (splits[i].split > split) {
            split_buffer[splits[i].primitive] = SPLIT_RIGHT;
            }
	    }
    }

    for (UINT32 i = 0; i < primitives.size(); ++i) {
        UINT32 primitive = primitives[i];
        if (split_buffer[primitive] == SPLIT_BOTH) {
            left.push_back(primitive);
            right.push_back(primitive);
	    } else if (split_buffer[primitive] == SPLIT_LEFT) {
            left.push_back(primitive);
	    } else if (split_buffer[primitive] == 2) {
            right.push_back(primitive);
	    }
    }
}

void KdTree::BuildTree(const AABox& bb, UINT32 depth, UINT32 curr, const vector<UINT32>& primitives) {

    FLOAT split, tsa, inv_tsa;
    KdDim split_dim = DIM_INVALID;
    FLOAT cost = INFINITY;
    vector<UINT32> leftPrimitives;
    vector<UINT32> rightPrimitives;
    KdSplitPlane* splits;
    UINT32 num_splits[3];
    UINT tL, tR;

    assert(bb.min[0] <= bb.max[0]);
    assert(bb.min[1] <= bb.max[1]);
    assert(bb.min[2] <= bb.max[2]);

    if (depth < KD_MAX_DEPTH && primitives.size() > KD_MIN_LEAF) {
        tsa = bb.SurfaceArea();
	    inv_tsa = 1.0f / tsa;
	    assert(inv_tsa > 0);

	    for (int dim = 0; dim < 3; ++dim) {
	        splits = &(all_splits[dim][0]);
	        num_splits[dim] = ComputeSplitPlanes(splits, primitives, (KdDim)dim);

	        UINT32 n_left = 0;
            UINT32 n_right = primitives.size();
	        UINT32 n_plane = 0;

	        for (UINT32 i = 0; i < num_splits[dim];) {
		        FLOAT test_split = splits[i].split;
		        UINT32 pStart, pEnd, pPlane;
		        pStart = pEnd = pPlane = 0;

		        while (splits[i].split == test_split && 
			        splits[i].type == KdSplitPlane::EXIT &&
			        i < num_splits[dim]) {
		            ++pEnd;
		            ++i;
		        }
		        while (splits[i].split == test_split && 
			        splits[i].type == KdSplitPlane::PLANE &&
			        i < num_splits[dim]) {
		            ++pPlane;
		            ++i;
		        }
		        while (splits[i].split == test_split && 
			        splits[i].type == KdSplitPlane::ENTER &&
			        i < num_splits[dim]) {
		            ++pStart;
		            ++i;
		        }
		        n_plane = pPlane;
		        n_right -= pPlane;
		        n_right -= pEnd;
		        if (test_split < bb.max[dim] && test_split > bb.min[dim]) {
		            // compute SAH 
		            FLOAT test_cost = ComputeCost(bb, test_split, (KdDim)dim, n_left, n_right, n_plane, inv_tsa);
		            if (test_cost < cost) {
			            cost = test_cost;
			            split_dim = (KdDim)dim;
			            split = test_split;		
			            tL = n_left + n_plane;
			            tR = n_right + n_plane;
		            }
		        }
		        n_left += pStart;
		        n_left += pPlane;
		        n_plane = 0;
	        } 
	    } 

	    if (cost > primitives.size() * KD_ISECT_COST) {
		    split_dim = DIM_INVALID;	
        }
    }

    /* this node will be split and is an internal node */
    if (split_dim != DIM_INVALID) {
        PartitionTriangles(&all_splits[split_dim][0], num_splits[split_dim], primitives, 
            leftPrimitives, rightPrimitives, split);
        UINT32 lChild = m_nextNode;
        UINT32 rChild = m_nextNode + 1;
        build_tree[curr].children = 0;
        build_tree[curr].isLeaf = 0;
        build_tree[curr].offset = lChild - curr;
        build_tree[curr].dimension = split_dim;
        build_tree[curr].split = split;
	    m_nextNode += 2;

        AABox child = bb;
	    child.max[split_dim] = split;
        BuildTree(child, depth + 1, lChild, leftPrimitives);
	    child = bb;
	    child.min[split_dim] = split;
        BuildTree(child, depth + 1, rChild, rightPrimitives);
    } else {
        m_totalChildren += primitives.size();
	    UINT32* children = (UINT32*) malloc(sizeof(UINT32) * primitives.size());
	    assert((int)children % 4 == 0);
        memcpy(children, &primitives[0], sizeof(UINT32) * primitives.size());
        build_tree[curr].children = children;
        build_tree[curr].isLeaf = 1;
        build_tree[curr].num_children = primitives.size();
    }
}

}
