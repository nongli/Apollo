/**
 * This is model for a sphere.
 */

#pragma once

#include "ApolloEngine.h"
#include "Model.h"
#include "Primitive.h"

namespace Apollo {
    class Sphere : public Primitive, public Model {
	public:
	    Sphere();
	    Sphere(const Vector3& center, FLOAT radius);
	    Sphere(const Sphere& s);

	    virtual void Init();

	    virtual ~Sphere();

	    virtual bool Intersect(const Ray&, Intersection&) const;
	    virtual bool Intersect(const Ray&) const;

	    virtual void GetAABoundingBox(AABox&) const;
	   
        virtual UINT32 GetNumPrimitives() const { return 1; }
	    virtual void GetPrimitives(std::vector<const Primitive*>&) const;
	    
	    virtual void GetNormalAndUV(SurfaceElement&) const;

	    
	    virtual void Transform(const OrthonormalBasis& basis);

	    virtual FLOAT SurfaceArea() const;

	public:
	    Vector3		center;
	    FLOAT		radius;
	    FLOAT		radius2;
    };

	inline Sphere::Sphere() : Primitive(nullptr), center(0, 0, 0), radius(1), radius2(1) {}
	inline Sphere::Sphere(const Vector3& c, FLOAT r) : Primitive(nullptr), center(c), radius(r), radius2(r*r) {}
	inline Sphere::Sphere(const Sphere& s) : Primitive(nullptr), center(s.center), radius(s.radius), radius2(s.radius2) {}
	inline Sphere::~Sphere() {}
	
    inline bool Sphere::Intersect(const Ray& ray, Intersection& in) const {	
        CHECK_MAILBOX_RAY(ray);
		Vector3 rayOrigin = ray.origin - center;
     
        DOUBLE B = rayOrigin.Dot(ray.direction);
        DOUBLE C = rayOrigin.Dot(rayOrigin) - radius2;
        DOUBLE D = B*B - C;
        
        if (D < 0) return NO_HIT;

        D = sqrt(D);
        DOUBLE t = -B - D;
        if (t > ray.t_min && t < in.t) {
            in.t = t;
            in.primitive = this;
            return HIT;
        }

        t = -B + D;
        if (t > ray.t_min && t < in.t) {
            in.t = t;
            in.primitive = this;
            return HIT;
        }

        return NO_HIT;        
    }

    inline bool Sphere::Intersect(const Ray& ray) const {
		Vector3 rayOrigin = ray.origin - center;
     
        DOUBLE B = rayOrigin.Dot(ray.direction);
        DOUBLE C = rayOrigin.Dot(rayOrigin) - radius2;
        DOUBLE D = B*B - C;
        
        if (D < 0) return NO_HIT;

        D = sqrt(D);
        DOUBLE t = -B - D;
        if (t > ray.t_min && t < ray.t_max) return HIT;

        t = -B + D;
        if (t > ray.t_min && t < ray.t_max) return HIT;

        return NO_HIT;   
    }    
    
    inline void Sphere::GetNormalAndUV(SurfaceElement& surfel) const {
		surfel.normal = surfel.iPoint - center;
		surfel.normal.Normalize();

		DOUBLE u, v;
		u = atan2(surfel.normal.y, surfel.normal.x);
		if (u < 0) u += APOLLO_2_PI;
		v = acos(surfel.normal.z) / APOLLO_PI;
		u /= APOLLO_2_PI;

		surfel.uv.u = (FLOAT)u;
		surfel.uv.v = (FLOAT)v;
	}

    inline FLOAT Sphere::SurfaceArea() const {
		return (FLOAT)(4 * APOLLO_PI * radius2);
    }
}
