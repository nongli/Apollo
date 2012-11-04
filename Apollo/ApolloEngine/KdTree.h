#pragma once
/**
 * This is an implemention of a kdtree acceleration data structure using the SAH.
 * Construction of this tree is O(n log^2(n))
 */

#include "Accel.h"
#include "AABox.h"

namespace Apollo {

    struct KdSplitPlane;
    enum KdDim;
    
    class KdTree : public Accel {
	public:
	    KdTree();
	    virtual ~KdTree();

		virtual void SetGeometry(std::vector<Model*>& models);
        virtual void AddGeometry(Model* model) { m_models.push_back(model); m_dirty = true; }
        virtual bool Init() { return Rebuild(); }
	    virtual bool Rebuild();

		virtual bool Intersect(const Ray&) const;
	    virtual bool Intersect(const Ray&, Intersection&) const;
		virtual bool Intersect(const Ray&, std::vector<Intersection>&) const;
	
    private:
        void BuildTree(const AABox& bb, UINT32 depth, UINT32 curr, const std::vector<UINT>& primitives);
        UINT32 ComputeSplitPlanes(KdSplitPlane* splits, const std::vector<UINT32>& primitives, KdDim dim);
		void CompressTree2();

	private:
        struct KdNode* m_root;
        AABox m_bounds;
        UINT32 m_byte_size;
        UINT32 m_next_node;
        UINT32 m_total_children;
        bool m_dirty;
		std::vector<const Primitive*> m_primitives;

		// Primitives that are not bounded (e.g. infinite plane) and not stored in the tree structure
		std::vector<const Primitive*> m_unbounded_primitives;

        std::vector<UINT> m_primitives_index;
		std::vector<Model*>	m_models;
    };
};


