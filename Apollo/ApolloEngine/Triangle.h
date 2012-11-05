#pragma once

/* Base triangle primitive.  Triangle uses baryocentric coordinates for
 * intersection.  The triangle is precomputed to optimize intersections */

#include "Primitive.h"

namespace Apollo {   
    class Mesh;

    static BYTE mod5[] = {0, 1, 2, 0, 1};
    
    class Triangle : public Primitive {
    public:
        struct Data {
	        UINT32 A, B, C;			//Indices into mesh vertex list
	        UINT32 nA, nB, nC;	    //Indices into mesh normal list
	        UINT32 tA, tB, tC;	    //Indices into mesh texture list
        };
	public:	
	    Triangle(Mesh*);
        Triangle(Mesh* mesh, const Data& data);

	    /* Triangle specific methods */
	    void SetVertices(UINT32 v1, UINT32 v2, UINT32 v3);
	    void SetNormals(UINT32 n1, UINT32 n2, UINT32 n3);
	    void SetUVs(UINT32 t1, UINT32 t2, UINT32 t3);

	    void Init();
	    ~Triangle();

	    /* Primitive methods implemented */
	    void GetAABoundingBox(AABox&) const;
	    bool Intersect(const Ray&) const;
	    bool Intersect(const Ray&, Intersection&) const;
	    void GetNormalAndUV(SurfaceElement&) const;
	    void Transform(const OrthonormalBasis& basis);
	    float SurfaceArea() const;
	
	protected:
        Data data;
	    Mesh* mesh;

	private:
	    DOUBLE n_u, n_v, n_d;
	    DOUBLE b_u, b_v, b_d;
	    DOUBLE c_u, c_v, c_d;
	    UINT32 k;
    };

    inline bool Triangle::Intersect(const Ray& ray) const {
		#define ku mod5[k + 1]
		#define kv mod5[k + 2]

		const DOUBLE nd = 1.0 / (ray.direction[k] + 
						         n_u * ray.direction[ku] +
						         n_v * ray.direction[kv]);

		const DOUBLE t = (n_d - ray.origin[k] -
				          n_u * ray.origin[ku] - 
				          n_v * ray.origin[kv]) * nd;

		if (t > ray.t_max || t < ray.t_min) return NO_HIT;

		const DOUBLE hu = ray.origin[ku] + t*ray.direction[ku];
		const DOUBLE hv = ray.origin[kv] + t*ray.direction[kv];
		const DOUBLE lambda = hu*b_u +  hv*b_v + b_d;
		
		if (lambda < -EPSILON) return NO_HIT;

		const DOUBLE mue = hu*c_u + hv*c_v + c_d;

		if (mue < -EPSILON) return NO_HIT;
		if (lambda + mue > 1.0) return NO_HIT;

		return HIT;
    }

    inline bool Triangle::Intersect(const Ray& ray, Intersection& i) const {
        CHECK_MAILBOX_RAY(ray);
		#define ku mod5[k + 1]
		#define kv mod5[k + 2]

		const DOUBLE nd = 1.0 / (ray.direction[k] + 
					 n_u * ray.direction[ku] +
					 n_v * ray.direction[kv]);

		const DOUBLE t = (n_d - ray.origin[k] -
				  n_u * ray.origin[ku] - 
				  n_v * ray.origin[kv]) * nd;

		if (t > i.t || t < ray.t_min) return NO_HIT;

		const DOUBLE hu = ray.origin[ku] + t * ray.direction[ku];
		const DOUBLE hv = ray.origin[kv] + t * ray.direction[kv];
		const DOUBLE lambda = hu*b_u +  hv*b_v + b_d;

		if (lambda < -EPSILON) return NO_HIT;

		const DOUBLE mue = hu * c_u + hv * c_v + c_d;

		if (mue < -EPSILON) return NO_HIT;
		if (lambda + mue > 1.0) return NO_HIT;

		//HIT
		i.t		= t;
		i.primitive	= this;
		i.u		= (FLOAT)lambda;
		i.v		= (FLOAT)mue;

		return HIT;
    }
}
