#pragma once

#include "EngineCommon.h"

namespace Apollo {
	class Primitive;

	struct Ray {
		Vector3	    origin;
		Vector3	    direction;
		Vector3	    invDirection;
		BYTE		sign[3];
		DOUBLE	    t_min, t_max;
		OBJECT_ID   id;
		BYTE		depth;

		Ray() {};

		Ray(const Vector3& o, const Vector3& d, BYTE depth=0) { 
			Set(o, d, depth);
		};

		void Set(const Vector3& o, const Vector3& d, BYTE _depth=0) {
			origin			= o;
            direction		= d.GetDirection();

			invDirection.x	= 1.f / direction.x;
			invDirection.y	= 1.f / direction.y;
			invDirection.z	= 1.f / direction.z;

			sign[0]			= (invDirection.x < 0);
			sign[1]			= (invDirection.y < 0);
			sign[2]			= (invDirection.z < 0);

			t_min			= EPSILON;
			t_max			= INFINITY;

			depth			= _depth;

			id				= GetLUID();
		}
	};

	struct Intersection {
		DOUBLE				t;
		const Primitive*	primitive;
		FLOAT				u;
		FLOAT				v;

		Intersection() 
		{
			Reset();
		}

        void Reset()
        {
            t = INFINITY;
        }
    };

    struct SurfaceElement {
		const Intersection*		intersection;
		const Ray*				ray;
		Vector3					iPoint;
		Vector3					normal;
		UV						uv;

		void Init(const Intersection* i, const Ray* r);
    };
}
