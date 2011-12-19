/**
 * This is model for a InfinitePlane.
 */

#pragma once

#include "EngineCommon.h"
#include "Model.h"
#include "Primitive.h"
#include "AABox.h"

namespace Apollo {
    class InfinitePlane : public Primitive, public Model {
	public:
	    InfinitePlane();

		// Construct a InfinitePlane passing it a point on the InfinitePlane and a normal.

	    InfinitePlane(const Vector3& v, const Vector3& normal);
	    InfinitePlane(const InfinitePlane& p);

	    virtual void Init();

	    virtual ~InfinitePlane();

	    virtual bool Intersect(const Ray&, Intersection&) const;
	    virtual bool Intersect(const Ray&) const;

	    virtual void GetAABoundingBox(AABox&) const;
	    
        virtual UINT32 GetNumPrimitives() const { return 1; }
	    virtual void GetPrimitives(std::vector<const Primitive*>&) const;
	    
	    virtual void GetNormalAndUV(SurfaceElement&) const;
	    
	    virtual void Transform(const OrthonormalBasis& basis);

	    virtual FLOAT SurfaceArea() const;

	public:
	    Vector3			center;
	    Vector3			normal;
	    DOUBLE			dist;
    };

	inline InfinitePlane::InfinitePlane() : Primitive(nullptr) {
		normal = Vector3::YVEC();
		center = Vector3(0, 0, 0);
		dist   = 0;
	}

	inline InfinitePlane::InfinitePlane(const Vector3& v, const Vector3& n) : Primitive(nullptr) {
		normal	= n.GetDirection();
		center	= v;
		dist	= -center.Dot(normal);
	}

	inline InfinitePlane::InfinitePlane(const InfinitePlane& p) : Primitive(nullptr) {
		normal = p.normal;
		center = p.center;
		dist   = p.dist;
	}

	inline InfinitePlane::~InfinitePlane() {}

    inline bool InfinitePlane::Intersect(const Ray& ray, Intersection& in) const {
		DOUBLE num  = - (ray.origin.Dot(normal) + dist);
		DOUBLE den  = ray.direction.Dot(normal);
		if (EQ(den, 0)) return NO_HIT;

        DOUBLE t = num / den;
		if (t < ray.t_min || t > in.t) return NO_HIT;

		in.t = t;
		in.primitive = this;
		return HIT;
    }

    inline bool InfinitePlane::Intersect(const Ray& ray) const {
		DOUBLE den  = ray.direction.Dot(normal);
		if (EQ(den, 0)) return NO_HIT;
        
		DOUBLE num  = - (ray.origin.Dot(normal) + dist);
		DOUBLE t    = num / den;
		
		if (t < ray.t_min || t > ray.t_max) return NO_HIT;
		return HIT;
    }    
    
    inline void InfinitePlane::GetNormalAndUV(SurfaceElement& surfel) const {
        // TODO: not quite right (uv)
 		surfel.normal = normal;
        surfel.uv = UV(0, 0);
    }

    inline FLOAT InfinitePlane::SurfaceArea() const {
		return 0;
    }
}
