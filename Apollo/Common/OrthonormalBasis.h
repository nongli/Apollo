#pragma once

#include "Vector3.h"

namespace Apollo 
{
    class OrthonormalBasis 
	{
	
	public:
	    Vector3 u, v, n;

	    OrthonormalBasis();

	    OrthonormalBasis(const Vector3& u,
						 const Vector3& v,
						 const Vector3& n);

	    void Set(const Vector3& u,
				 const Vector3& v,
				 const Vector3& n);

	    void FlipU();
	    void FlipV();
	    void FlipN();

	    Vector3& TransformToWorld(Vector3& r) const;
	    Vector3& TransformToObject(Vector3& r) const;

    };

    inline OrthonormalBasis::OrthonormalBasis() 
	{
		u = Vector3(1, 0, 0);
		v = Vector3(0, 1, 0);
		n = Vector3(0, 0, 1);
    }

    inline OrthonormalBasis::OrthonormalBasis(const Vector3& u_,
				       const Vector3& v_,
				       const Vector3& n_) 
	{
		u = u_;
		v = v_;
		n = n_;
    }

    inline void OrthonormalBasis::Set(const Vector3& u_,
									  const Vector3& v_,
									  const Vector3& n_) 
	{
		u = u_;
		v = v_;
		n = n_;
    }


    inline void OrthonormalBasis::FlipU() 
	{
		u = -u;
    }

    inline void OrthonormalBasis::FlipV() 
	{
		v = -v;
    }

    inline void OrthonormalBasis::FlipN() 
	{
		n = -n;
    }

    inline Vector3& OrthonormalBasis::TransformToObject(Vector3& v_) const 
	{
		DOUBLE x = v_.x*u.x + v_.y*u.y + v_.z*u.z;
		DOUBLE y = v_.x*v.x + v_.y*v.y + v_.z*v.z;
		DOUBLE z = v_.x*n.x + v_.y*n.y + v_.z*n.z;
	
		v_.x = x;
		v_.y = y;
		v_.z = z;
		return v_;
    }

    inline Vector3& OrthonormalBasis::TransformToWorld(Vector3& v_) const 
	{
		DOUBLE x = v_.x*u.x + v_.y*v.x + v_.z*n.x;
		DOUBLE y = v_.x*u.y + v_.y*v.y + v_.z*n.y;
		DOUBLE z = v_.x*u.z + v_.y*v.z + v_.z*n.z;

		v_.x = x;
		v_.y = y;
		v_.z = z;
		return v_;
    }
};

