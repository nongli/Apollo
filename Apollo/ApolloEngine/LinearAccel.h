#pragma once
/**
 * This is a very simple data acceleration data structure to store the
 * primitives.  It stores all of the primitives in a vector and has
 * expected O(n) performance for intersections.
 */

#include "Accel.h"

namespace Apollo {

    class LinearAccel : public Accel {
	public:
	    LinearAccel();
	    virtual ~LinearAccel();

		virtual void SetGeometry(std::vector<Model*>& models);
		virtual void AddGeometry(Model* model);
	    virtual bool Init();
	    virtual bool Rebuild();

		virtual bool Intersect(const Ray&) const;
	    virtual bool Intersect(const Ray&, Intersection&) const;
		virtual bool Intersect(const Ray&, std::vector<Intersection>&) const;
	
	private:
		std::vector<const Primitive*>	m_primitives;
		std::vector<Model*>				m_models;
    };
};


