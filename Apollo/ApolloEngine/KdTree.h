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

	private:
        struct KdNode* m_root;
        AABox m_bounds;
        UINT32 m_size;
        UINT32 m_nextNode;
        UINT32 m_totalChildren;
        bool m_dirty;
		std::vector<const Primitive*> m_primitives;
        std::vector<UINT> m_primitivesIndex;
		std::vector<Model*>	m_models;
    };
};


