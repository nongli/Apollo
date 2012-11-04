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
#define STACK_SIZE 256

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

	bool operator< (const KdSplitPlane& p) const {
		if (split < p.split) return true;
		if (split > p.split) return false;
		return type < p.type;	
	}

	bool operator== (const KdSplitPlane& p) const {
		return EQ(split, p.split) && dim == p.dim && type == p.type;
	}

	string DebugString() const {
		stringstream ss;
		ss << "Split: " << split 
		   << " Dim: " << dim
		   << " Type: " << type;
		return ss.str();
	}
};


struct KdStackNode {
    KdNode* node;
    double t_near;
    double t_far;
};


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
    m_byte_size = 0;
    if (m_root) free(m_root);
    m_root = nullptr;

    m_build_tree.resize(KD_START_SIZE);

    // Find bounding box
    AABox primitiveBox;
    for (UINT32 i = 0; i < m_primitives.size(); ++i) {
        m_primitives[i]->GetAABoundingBox(primitiveBox);
		if (primitiveBox.IsUnbounded()) {
			m_unbounded_primitives.push_back(m_primitives[i]);
			continue;
		}
        m_bounds.Add(primitiveBox);
    }

    // Allocate split plane memory
    for (UINT32 i = 0; i < 3; ++i) {
        m_all_splits[i].resize(2 * m_primitives.size());
    }
    m_split_buffer.resize(m_primitives.size());
    m_primitives_index.resize(m_primitives.size());
    for (UINT32 i = 0; i < m_primitives.size(); ++i) {
        m_primitives_index[i] = i;
    }

	m_total_children = 0;
    m_next_node = 2;
    BuildTree(m_bounds, 0, 1, m_primitives_index);
	CompactTree();

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
	        splits[n].type = KdSplitPlane::PLANE;
	        n++;
	    } else {
	        splits[n].dim = dim;
	        splits[n].type = KdSplitPlane::ENTER;
            splits[n].split = (FLOAT)box.min[dim];
	        splits[n].primitive = primitives[i];
	        n++;

	        splits[n].dim = dim;
	        splits[n].primitive = primitives[i];
	        splits[n].split = (FLOAT)box.max[dim];
	        splits[n].type = KdSplitPlane::EXIT;
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

void KdTree::PartitionTriangles(const KdSplitPlane* splits, UINT32 n, const vector<UINT32>& primitives,
    vector<UINT32>& left, vector<UINT32>& right, float split) {

    const char SPLIT_BOTH = 0;
    const char SPLIT_LEFT = 1;
    const char SPLIT_RIGHT = 2;

    memset(&m_split_buffer[0], SPLIT_BOTH, sizeof(char) * m_split_buffer.size());

    for (UINT32 i = 0; i < n; ++i) {
	    if (splits[i].type == KdSplitPlane::EXIT && splits[i].split <= split) {
            m_split_buffer[splits[i].primitive] = SPLIT_LEFT;
	    } else if (splits[i].type == KdSplitPlane::ENTER && splits[i].split >= split) {
            m_split_buffer[splits[i].primitive] = SPLIT_RIGHT;
	    } else if (splits[i].type == KdSplitPlane::PLANE) {
	        if (splits[i].split < split) {
                m_split_buffer[splits[i].primitive] = SPLIT_LEFT;
            } else if (splits[i].split > split) {
				m_split_buffer[splits[i].primitive] = SPLIT_RIGHT;
            }
	    }
    }

    for (UINT32 i = 0; i < primitives.size(); ++i) {
        UINT32 primitive = primitives[i];
        if (m_split_buffer[primitive] == SPLIT_BOTH) {
            left.push_back(primitive);
            right.push_back(primitive);
	    } else if (m_split_buffer[primitive] == SPLIT_LEFT) {
            left.push_back(primitive);
	    } else if (m_split_buffer[primitive] == SPLIT_RIGHT) {
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
	        splits = &(m_all_splits[dim][0]);
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
        PartitionTriangles(&m_all_splits[split_dim][0], num_splits[split_dim], primitives, 
            leftPrimitives, rightPrimitives, split);
        UINT32 lChild = m_next_node;
        UINT32 rChild = m_next_node + 1;
        m_build_tree[curr].children = 0;
        m_build_tree[curr].isLeaf = 0;
        m_build_tree[curr].offset = lChild - curr;
        m_build_tree[curr].dimension = split_dim;
        m_build_tree[curr].split = split;
	    m_next_node += 2;

        AABox child = bb;
	    child.max[split_dim] = split;
        BuildTree(child, depth + 1, lChild, leftPrimitives);
	    child = bb;
	    child.min[split_dim] = split;
        BuildTree(child, depth + 1, rChild, rightPrimitives);
    } else {
        m_total_children += primitives.size();
		if (primitives.size() != 0) {
			UINT32* children = (UINT32*) malloc(sizeof(UINT32) * primitives.size());
			assert((int)children % 4 == 0);
			memcpy(children, &primitives[0], sizeof(UINT32) * primitives.size());
			m_build_tree[curr].children = children;
		} else {
			m_build_tree[curr].children = NULL;
		}
        m_build_tree[curr].isLeaf = 1;
        m_build_tree[curr].num_children = primitives.size();
    }
}

void KdTree::CompactTree() {
	int num_nodes = m_next_node;
	m_byte_size = sizeof(KdNode) * num_nodes + m_total_children * sizeof(UINT32);
	m_root = (KdNode*)malloc(m_byte_size);

    void* cStart = ((char*)m_root) + sizeof(KdNode) * num_nodes;
    int* children = (int*) cStart;

    for (unsigned int i = 1; i < num_nodes; ++i) {
		if (m_build_tree[i].isLeaf) {
			memcpy(children, m_build_tree[i].children, sizeof(int)*m_build_tree[i].num_children);
			int offset = ((char*)children - (char*)&m_root[i]);
			assert(offset > 0);
			KD_INIT_LEAF(m_root[i], m_build_tree[i].num_children, offset);
			children += m_build_tree[i].num_children;
		} else {
			int offset = m_build_tree[i].offset * sizeof(KdNode);
			int dim = m_build_tree[i].dimension;
			float s = m_build_tree[i].split;
			assert(offset > 0);
			KD_INIT_INTERNAL(m_root[i], dim, offset, s);
		}
    }

    for (unsigned int i = 1; i < num_nodes; ++i) {
		if (m_build_tree[i].isLeaf) free(m_build_tree[i].children);
	}

	m_build_tree.clear();
	m_all_splits[0].clear();
	m_all_splits[1].clear();
	m_all_splits[2].clear();
	m_split_buffer.clear();
}

bool KdTree::Intersect(const Ray& ray) const {
	for (size_t i = 0; i < m_unbounded_primitives.size(); ++i) {
		if (m_unbounded_primitives[i]->Intersect(ray)) return HIT;
	}
	
	// Intersect with the bounding box for the kd tree
	if (!m_bounds.Intersect(ray)) return NO_HIT;

    KdNode* node = &m_root[1];
    KdStackNode stack[STACK_SIZE];
    int stack_size = 0;
    
	double t_near = ray.t_min;
	double t_far = ray.t_max;

    // Has intersected with bounding box, continue traversing tree
    while (true) {
		// if the node is a leaf, intersect all its primitives
		while (!KD_ISLEAF(node)) {
			// the ray is perpendicular to the split plane 
			if (EQ(ray.direction[KD_DIM(node)], 0)) { 
				node = KD_CHILD(node, ray.origin[KD_DIM(node)] > (node -> split));
			} else {
				double d = (node->split - ray.origin[KD_DIM(node)]) * ray.invDirection[KD_DIM(node)];
				// cull the children 
				if (d <= t_near) {
					// continue with far child
					node = KD_CHILD(node, ray.direction[KD_DIM(node)] > 0);
				} else if (d >= t_far) {
					//continue with near child
					node = KD_CHILD(node, ray.direction[KD_DIM(node)] < 0);
				} else {
					// traverse both children
					assert(stack_size < STACK_SIZE);
					stack[stack_size].node = KD_CHILD(node, ray.direction[KD_DIM(node)] > 0);
					stack[stack_size].t_near = d;
					stack[stack_size].t_far = t_far;
					++stack_size;
					node = KD_CHILD(node, ray.direction[KD_DIM(node)] < 0);
					t_far = d;
				}
			}
		} //end internal node traversal

		// intersect with leaf 
		int* first_child = (int*)(((char*)node) + KD_OFFSET(node));
		for (unsigned int i = 0; i < node->num_children; ++i) {
			if (m_primitives[first_child[i]]->Intersect(ray)) return HIT;
		}

		// Finished traversing all nodes
		if (!stack_size) break;

		// pop from stack 
		--stack_size;
		node = stack[stack_size].node;
		t_near = stack[stack_size].t_near;
		t_far = stack[stack_size].t_far;
    }

	return NO_HIT;
}

bool KdTree::Intersect(const Ray& ray, Intersection& inter) const {
	bool hit = NO_HIT;
	for (size_t i = 0; i < m_unbounded_primitives.size(); ++i) {
		hit |= m_unbounded_primitives[i]->Intersect(ray, inter);
	}

    // Intersect with the bounding box for kd tree
	if (!m_bounds.Intersect(ray)) return hit;

    KdNode* node = &m_root[1];
    KdStackNode stack[STACK_SIZE];
    int stack_size = 0;
    
	double t_near = ray.t_min;
	double t_far = ray.t_max;

    // Has intersected with bounding box, continue traversing tree
    while (true) {
		// if the node is a leaf, intersect all its primitives
		while (!KD_ISLEAF(node)) {
			// the ray is perpendicular to the split plane 
			if (EQ(ray.direction[KD_DIM(node)], 0)) { 
				node = KD_CHILD(node, ray.origin[KD_DIM(node)] > (node -> split));
			} else {
				double d = (node->split - ray.origin[KD_DIM(node)]) * ray.invDirection[KD_DIM(node)];
				// cull the children 
				if (d <= t_near) {
					// continue with far child
					node = KD_CHILD(node, ray.direction[KD_DIM(node)] > 0);
				} else if (d >= t_far) {
					//continue with near child
					node = KD_CHILD(node, ray.direction[KD_DIM(node)] < 0);
				} else {
					// traverse both children
					assert(stack_size < STACK_SIZE);
					stack[stack_size].node = KD_CHILD(node, ray.direction[KD_DIM(node)] > 0);
					stack[stack_size].t_near = d;
					stack[stack_size].t_far = t_far;
					++stack_size;
					node = KD_CHILD(node, ray.direction[KD_DIM(node)] < 0);
					t_far = d;
				}
			}
		} //end internal node traversal

		// intersect with leaf 
		int* first_child = (int*)(((char*)node) + KD_OFFSET(node));
		for (unsigned int i = 0; i < node->num_children; ++i) {
			hit |= m_primitives[first_child[i]]->Intersect(ray, inter);
		}

		// early termination 
		if (inter.t < t_far) break;

		// Finished traversing all nodes
		if (!stack_size) break;

		// pop from stack 
		--stack_size;
		node = stack[stack_size].node;
		t_near = stack[stack_size].t_near;
		t_far = stack[stack_size].t_far;
    }

	return hit;
}

bool KdTree::Intersect(const Ray& ray, vector<Intersection>& inters) const {
	ApolloException::NotYetImplemented();
	return NO_HIT;
}

}
