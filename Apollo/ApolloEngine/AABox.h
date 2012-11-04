/**
 * This is an axis aligned  box.  It is represented by specifying
 * the minimum and maximum values in each dimension 
 */

#pragma once

#include "Model.h"
#include "Primitive.h"

namespace Apollo {
    class AABox : public Primitive, public Model {
	public:
	    AABox();
	    AABox(DOUBLE minx, DOUBLE miny, DOUBLE minz,
			  DOUBLE maxx, DOUBLE maxy, DOUBLE maxz);
	    AABox(DOUBLE min[3], DOUBLE max[3]);
	    AABox(const AABox& b);

	    virtual void Init();

	    virtual ~AABox();

	    /* Primitive and Model methods */
	    virtual bool Intersect(const Ray&, Intersection&) const;
	    virtual bool Intersect(const Ray&) const;

	    virtual void GetAABoundingBox(AABox&) const;
	   
        virtual UINT32 GetNumPrimitives() const { return 1; }
	    virtual void GetPrimitives(std::vector<const Primitive*>&) const;
	   
	    virtual void GetNormalAndUV(SurfaceElement&) const;

	    virtual void Transform(const OrthonormalBasis& basis);

	    virtual FLOAT SurfaceArea() const;

	    // Box specific methods
		void Add(const Vector3& v);
	    void Add(const Primitive& p);
	    void Add(const Model& model);
	    void Add(const AABox& box);

	    AABox& operator+= (const Vector3&);
	    AABox& operator+= (const Primitive&);
	    AABox& operator+= (const Model&);
	    AABox& operator+= (const AABox&);

	    void Set(const DOUBLE min[3], const DOUBLE max[3]);
	    AABox& operator= (const AABox& b);

		bool IsUnbounded() const {
			return min[0] == -INFINITY || min[1] == -INFINITY || min[2] == -INFINITY ||
				   max[0] == INFINITY || max[1] == INFINITY || max[2] == INFINITY;
		}

		void Reset() {
			min[0] = min[1] = min[2] = INFINITY;    
			max[0] = max[1] = max[2] = -INFINITY;    
			span[0] = span[1] = span[2] = 0;
		}

#pragma warning( disable : 4201 )
	    union {
			struct {
				DOUBLE min[3];
				DOUBLE max[3];
			};
			struct {
				DOUBLE data[6];
			};
	    };
#pragma warning (default : 4201)
	    DOUBLE span[3];
    };

	inline AABox::AABox() : Primitive(nullptr) {
		Reset();
	}

	inline AABox::AABox(DOUBLE minx, DOUBLE miny, DOUBLE minz,
				DOUBLE maxx, DOUBLE maxy, DOUBLE maxz) : Primitive(nullptr) {
		min[0] = minx;
		min[1] = miny;
		min[2] = minz;
		max[0] = maxx;
		max[1] = maxy;
		max[2] = maxz;

		span[0] = max[0] - min[0];
		span[1] = max[1] - min[1];
		span[2] = max[2] - min[2];
	}

	inline AABox::AABox(DOUBLE mi[3], DOUBLE ma[3]) : Primitive(nullptr) {
		min[0] = mi[0];
		min[1] = mi[1];
		min[2] = mi[2];
		max[0] = ma[0];
		max[1] = ma[1];
		max[2] = ma[2];
	    
		span[0] = max[0] - min[0];
		span[1] = max[1] - min[1];
		span[2] = max[2] - min[2];
	}

	inline AABox::AABox(const AABox& b) : Primitive(nullptr) {
		min[0] = b.min[0];
		min[1] = b.min[1];
		min[2] = b.min[2];
		max[0] = b.max[0];
		max[1] = b.max[1];
		max[2] = b.max[2];
	    
		span[0] = max[0] - min[0];
		span[1] = max[1] - min[1];
		span[2] = max[2] - min[2];
	}

	inline AABox::~AABox() {}

    inline void AABox::Set(const DOUBLE mi[3], const DOUBLE ma[3]) {
		min[0] = mi[0];
		min[1] = mi[1];
		min[2] = mi[2];
		max[0] = ma[0];
		max[1] = ma[1];
		max[2] = ma[2];

		span[0] = max[0] - min[0];
		span[1] = max[1] - min[1];
		span[2] = max[2] - min[2];
    }

    inline AABox& AABox::operator=(const AABox& b) {
        Set(b.min, b.max);
		return *this;
    }

    inline AABox& AABox::operator+= (const Vector3& v) {
		Add(v);
		return *this;
    }

    inline AABox& AABox::operator+= (const Primitive& p) {
		Add(p);
		return *this;
    }

    inline AABox& AABox::operator+= (const Model& m) {
		Add(m);
		return *this;
    }

    inline AABox& AABox::operator+= (const AABox& b) {
		Add(b);
		return *this;
    }

    inline void AABox::Add(const Vector3& p) {
		if (p[0] < min[0]) {
			min[0] = p[0];
			span[0] = max[0] - min[0];
		}		
		if (p[0] > max[0]) {
			max[0] = p[0];
			span[0] = max[0] - min[0];
		}

		if (p[1] < min[1]) {
			min[1] = p[1];
			span[1] = max[1] - min[1];
		}		
		if (p[1] > max[1]) {
			max[1] = p[1];
			span[1] = max[1] - min[1];
		}

		if (p[2] < min[2]) {
			min[2] = p[2];
			span[2] = max[2] - min[2];
		}
		if (p[2] > max[2]) {
			max[2] = p[2];
			span[2] = max[2] - min[2];
		}
    }

    inline void AABox::Add(const AABox& b) {
		if (b.min[0] < min[0]) {
			min[0]  = b.min[0];
			span[0] = max[0] - min[0];
		}
		if (b.max[0] > max[0]) {
			max[0]  = b.max[0];
			span[0] = max[0] - min[0];
		}

		if (b.min[1] < min[1]) {
			min[1]  = b.min[1];
			span[1] = max[1] - min[1];
		}
		if (b.max[1] > max[1]) {
			max[1]  = b.max[1];
			span[1] = max[1] - min[1];
		}

		if (b.min[2] < min[2]) {
			min[2]  = b.min[2];
			span[2] = max[2] - min[2];
		}
		if (b.max[2] > max[2]) {
			max[2]  = b.max[2];
			span[2] = max[2] - min[2];
		}
    }

    inline void AABox::Add(const Primitive& p) {
		AABox b;
		p.GetAABoundingBox(b);
		Add(b);
    }

    inline void AABox::Add(const Model& m) {
		AABox b;
		m.GetAABoundingBox(b);
		Add(b);
    }

    inline bool AABox::Intersect(const Ray& r, Intersection& in) const {
		DOUBLE t_n, t_f;
		DOUBLE t_near = r.t_min;
		DOUBLE t_far = in.t;
		for (BYTE i = 0; i < 3; ++i) {
			if (r.sign[i]) {
				t_n = (max[i] - r.origin[i]) * r.invDirection[i];
				t_f = (min[i] - r.origin[i]) * r.invDirection[i];
			} else {
				t_n = (min[i] - r.origin[i]) * r.invDirection[i];
				t_f = (max[i] - r.origin[i]) * r.invDirection[i];
			}

			t_near = MAX(t_n, t_near);
			t_far  = MIN(t_f, t_far);

			if (t_near > t_far) return NO_HIT;
		}
		in.primitive	= this;
		in.t			= t_near;	
		return HIT;
    }

    inline bool AABox::Intersect(const Ray& r) const {
		DOUBLE tmin, tmax, tymin, tymax, tzmin, tzmax;

		tmin  = (data[r.sign[0]*3] - r.origin.x) * r.invDirection.x;
		tmax  = (data[(1-r.sign[0])*3] - r.origin.x) * r.invDirection.x;
		tymin = (data[r.sign[1]*3 + 1] - r.origin.y) * r.invDirection.y; 
		tymax = (data[(1-r.sign[1])*3 + 1] - r.origin.y) * r.invDirection.y; 

		if ( (tmin > tymax) || (tymin > tmax) ) return NO_HIT;
		
        tmin = MAX(tmin, tymin);
		tmax = MIN(tmax, tymax);

		tzmin = (data[r.sign[2]*3 + 2] - r.origin.z) * r.invDirection.z; 
		tzmax = (data[(1-r.sign[2])*3 + 2] - r.origin.z) * r.invDirection.z; 

		if ( (tmin > tzmax) || (tzmin > tmax) ) return NO_HIT;

		tmin = MAX(tmin, tzmin);
		tmax = MIN(tmax, tzmax);

		return (tmin < r.t_max && tmax > r.t_min) ? HIT : NO_HIT;
    }

    inline void AABox::GetNormalAndUV(SurfaceElement& surfel) const {
		// Figure out which face it hit
		const Vector3& i = surfel.iPoint;
		surfel.normal = Vector3::ZERO();

		for (BYTE d = 0; d < 3; d++) {
			if (EQ(i[d], min[d])) {
				surfel.normal[d] = -1.0f;
				return;
			}
			if (EQ(i[d], max[d])) {
				surfel.normal[d] = 1.0f;
				return;
			}
		}
		// TODO assign UV
    }

    inline FLOAT AABox::SurfaceArea() const {
		FLOAT dx = (FLOAT)(max[0] - min[0]);
		FLOAT dy = (FLOAT)(max[1] - min[1]);
		FLOAT dz = (FLOAT)(max[2] - min[2]);
		return 2 * (dx*dy + dx*dz + dy*dz);
    }

}
