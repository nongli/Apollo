#pragma once

#include "Types.h"
#include "ApolloMath.h"
#include "Error.h"
#include "Macros.h"

namespace Apollo 
{
    class Vector3 
	{
	
	public:
		enum Dimension
		{
			X_AXIS = 0, Y_AXIS, Z_AXIS
		};

	public:
	    DOUBLE x, y, z;

	    Vector3();
	    ~Vector3();
	    Vector3(DOUBLE x, DOUBLE y, DOUBLE z);
	    Vector3(const DOUBLE d[3]);
	    Vector3(const Vector3& copy);

	    const DOUBLE& operator[] (int i) const;
	    DOUBLE& operator[] (int i);

	    inline operator DOUBLE* () { return (DOUBLE*) this; }
	    inline operator const DOUBLE*() const { return (DOUBLE*) this; }

	    Vector3& operator= (const Vector3& rhs);
	    bool operator== (const Vector3& rhs) const;
	    bool operator!= (const Vector3& rhs) const;
	    bool IsEqual(const Vector3& rhs) const;
	    bool IsNotEqual(const Vector3& rhs) const;

	    bool IsZero() const;
	    bool IsUnit() const;

	    Vector3& set(DOUBLE x, DOUBLE y, DOUBLE z);
		
	    Vector3 operator+ (const Vector3& rhs) const;
	    Vector3 operator- (const Vector3& rhs) const;
	    Vector3 operator* (DOUBLE scale) const;
	    Vector3 operator/ (DOUBLE scale) const;
	    Vector3 operator- () const;

	    Vector3& operator+= (const Vector3& rhs);
	    Vector3& operator-= (const Vector3& rhs);
	    Vector3& operator*= (DOUBLE scale);
	    Vector3& operator/= (DOUBLE scale);

	    DOUBLE Magnitude() const;
	    DOUBLE Magnitude2() const;  
	    DOUBLE Normalize();			
	    Vector3 GetDirection() const;

	    DOUBLE Sum() const;
	    DOUBLE Average() const;
		
	    Dimension MaxDimension() const;
		Dimension MinDimension() const;

	    Vector3 Rotate(const Vector3& src, const Vector3& dest) const;

	    // Assume normal is normalized, incident vector is exiting and
	    // resultant vector is exiting 
	    Vector3 ReflectVector(const Vector3& normal) const;
	    Vector3 RefractVector(const Vector3& normal, DOUBLE in, 
		    DOUBLE out = 1.0f) const;

	    DOUBLE Dot(const Vector3& rhs) const;
	    Vector3 Cross(const Vector3& rhs) const;
	    Vector3 Cross2(const Vector3& rhs1, const Vector3& rhs2) const;

	    static Vector3 Rand(DOUBLE max = 1.0, DOUBLE min = 0.0);

	    inline static const Vector3& ZERO() 
		{
			static Vector3 vec(0, 0, 0);
			return vec;
	    }

	    inline static const Vector3& UNIT() 
		{
			static Vector3 vec(1, 1, 1);
			return vec;
	    }

	    inline static const Vector3& XVEC() 
		{
			static Vector3 vec(1, 0, 0);
			return vec;
	    }

	    inline static const Vector3& YVEC() 
		{
			static Vector3 vec(0, 1, 0);
			return vec;
	    }
	    
		inline static const Vector3& ZVEC() 
		{
			static Vector3 vec(0, 0, 1);
			return vec;
	    }

    };

    Vector3 operator* (DOUBLE scale, const Vector3& vector);
};

#include "Vector3.inl"
