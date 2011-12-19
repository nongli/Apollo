#pragma once

#include "Vector3.h"

namespace Apollo 
{

inline Vector3::Vector3() : x(0), y(0), z(0) {}

inline Vector3::Vector3(DOUBLE _x, DOUBLE _y, DOUBLE _z) :
	    x(_x), y(_y), z(_z) {}

inline Vector3::Vector3(const DOUBLE f[3]) : x(f[0]), y(f[1]), z(f[2]) {}

inline Vector3::Vector3(const Vector3& copy) : x(copy.x), y(copy.y), z(copy.z) {}

inline Vector3::~Vector3() {}

inline const DOUBLE& Vector3::operator[](int i) const 
{
    return ((DOUBLE*)this)[i];
}

inline DOUBLE& Vector3::operator[](int i) 
{
    return ((DOUBLE*)this)[i];
}

inline Vector3& Vector3::operator=(const Vector3& rhs) 
{
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
    return *this;
}

inline Vector3& Vector3::set(DOUBLE _x, DOUBLE _y, DOUBLE _z) 
{
	x = _x;
	y = _y;
	z = _z;
	return *this;
}

inline bool Vector3::operator==(const Vector3& rhs) const 
{
    return (x == rhs.x && y == rhs.y && z == rhs.z);
}

inline bool Vector3::operator!=(const Vector3& rhs) const 
{
    return (x != rhs.x || y != rhs.y || z != rhs.z);
}

inline bool Vector3::IsEqual(const Vector3& rhs) const 
{
    return EQ(x, rhs.x) && EQ(y, rhs.y) && EQ(z, rhs.z);
}

inline bool Vector3::IsNotEqual(const Vector3& rhs) const 
{
    return NEQ(x, rhs.x) || NEQ(y, rhs.y) || NEQ(z, rhs.z);
}

inline bool Vector3::IsZero() const 
{
    return EQ(Magnitude2(), 0);
}

inline bool Vector3::IsUnit() const 
{
    return EQ(Magnitude2(), 1);
}

inline Vector3 Vector3::operator+(const Vector3& rhs) const 
{
    return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
}

inline Vector3 Vector3::operator-(const Vector3& rhs) const 
{
    return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
}

inline Vector3 Vector3::operator*(DOUBLE scale) const 
{
    return Vector3(x * scale, y * scale, z * scale);
}

inline Vector3 operator*(DOUBLE scale, const Vector3& vector) 
{
    return Vector3(scale * vector.x, scale * vector.y, scale * vector.z);
}

inline Vector3 Vector3::operator-() const 
{
    return Vector3(-x, -y, -z);
}

inline Vector3& Vector3::operator+= (const Vector3& rhs) 
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}

inline Vector3& Vector3::operator-= (const Vector3& rhs) 
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}

inline Vector3& Vector3::operator*= (DOUBLE scale) 
{
    x *= scale;
    y *= scale;
    z *= scale;
    return *this;
}

inline DOUBLE Vector3::Magnitude() const 
{
    return sqrt(x*x + y*y + z*z);
}

inline DOUBLE Vector3::Magnitude2() const 
{
    return x*x + y*y + z*z;
}

inline DOUBLE Vector3::Normalize() 
{
    DOUBLE m = Magnitude();
    DOUBLE inv = 1.f / m;
    x *= inv;
    y *= inv;
    z *= inv;
    return m;
}

inline Vector3 Vector3::GetDirection() const 
{
    DOUBLE inv = 1.f / Magnitude();
    return Vector3(x * inv, y * inv, z * inv);
}

inline DOUBLE Vector3::Sum() const 
{
    return x + y + z;
}

inline DOUBLE Vector3::Average() const 
{
    return (x + y + z) * .33333333;
}

inline DOUBLE Vector3::Dot(const Vector3& rhs) const 
{
    return x * rhs.x + y * rhs.y + z * rhs.z;
}

inline Vector3 Vector3::Cross(const Vector3& rhs) const 
{
    return Vector3(y * rhs.z - z * rhs.y,
	    		   z * rhs.x - x * rhs.z,
	    		   x * rhs.y - y * rhs.x);
}


inline Vector3 Vector3::ReflectVector(const Vector3& normal) const 
{
	DOUBLE dot = this->Dot(normal);
    return 2*dot*normal - *this;
}

inline Vector3 Vector3::Cross2(const Vector3& rhs1, 
	const Vector3& rhs2) const 
{
	ApolloException::NotYetImplemented();
    UNREFERENCED_PARAMETER(rhs1);
    UNREFERENCED_PARAMETER(rhs2);
    return nullptr;
}

inline Vector3::Dimension Vector3::MaxDimension() const 
{
    if ( x*x > y*y) 
		return x*x > z*z ? X_AXIS : Z_AXIS;
    return y*y > z*z ? Y_AXIS : Z_AXIS;
}

inline Vector3::Dimension Vector3::MinDimension () const 
{
    if (y*y < x*x) 
		return y*y < z*z ? Y_AXIS : Z_AXIS;
    return x*x < z*z ? X_AXIS : Z_AXIS;
}

inline Vector3 Vector3::Rand(DOUBLE max, DOUBLE min) 
{	
    DOUBLE d = max - min;
    return Vector3(RandDouble() * d + min, 
				   RandDouble() * d + min, 
				   RandDouble() * d + min);
}

}

