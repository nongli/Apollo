#include "Vector3.inl"

namespace Apollo 
{

Vector3& Vector3::operator/= (DOUBLE scale) 
{
    
	if (NEQ(scale, 0.0)) 
	{
		scale = 1.f / scale;
		x *= scale;
		y *= scale;
		z *= scale;
    }
    else 
	{
		x = INFINITY;
		y = INFINITY;
		z = INFINITY;
    }

    return *this;
}

Vector3 Vector3::operator/(DOUBLE scale) const 
{
    if (NEQ(0.0, scale)) 
	{
		scale = 1.f / scale;
		return Vector3(x * scale, y * scale, z * scale);
    }
    return Vector3(INFINITY, INFINITY, INFINITY);
}

Vector3 Vector3::Rotate(const Vector3& src, const Vector3& dest) const
{
	/*
    DOUBLE c = src.direction().dot(dest.direction());
    DOUBLE s = sqrt( 1 - (c*c));
    DOUBLE t = 1-c;

    Vector3 rot= src.direction().cross(dest.direction());

    Matrix4 quaternian(
	    t*rot.x*rot.x+c, t*rot.x*rot.y+s*rot.z, t*rot.x*rot.z-s*rot.y,0,
	    t*rot.x*rot.y-s*rot.z, t*rot.y*rot.y+c, t*rot.y*rot.z+s*rot.x,0,
	    t*rot.x*rot.z+s*rot.y, t*t*rot.z-s*rot.x, t*rot.z*rot.z+c,0,
	    0,0,0,1);

    return (*this*quaternian);
	*/
	ApolloException::NotYetImplemented();
    UNREFERENCED_PARAMETER(src);
    UNREFERENCED_PARAMETER(dest);
	return *this;
}

Vector3 Vector3::RefractVector(const Vector3& normal, DOUBLE in, DOUBLE out) const 
{    
    Vector3 norm;
    Vector3 dir = -GetDirection();
    DOUBLE n;

    if (this->Dot(normal) < 0) 
	{
		norm = normal;
		n = out / in;
    }
    else 
	{
		norm = -normal;
		n = in / out;
    }

    const DOUBLE d = dir.Dot(norm);
    const DOUBLE det = 1.f - n*n * (1 - d*d);

    if (det < 0)
		return Vector3(0, 0, 0);
    else 
		return -n * (dir - d*norm) - norm*sqrt(det); 
}

}

