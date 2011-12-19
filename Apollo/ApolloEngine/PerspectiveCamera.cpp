#include "PerspectiveCamera.h"

namespace Apollo 
{

PerspectiveCamera::PerspectiveCamera() : Camera() {
    m_pos		= Vector3(0, 0, 5);
    m_look		= Vector3(0, 0, -1);
    m_up		= Vector3(0, 1, 0); 

    m_width		= 400;
    m_height	= 400;

    m_fov		= 60.0f;
    m_ar		= 1.0f;

    Init();
    CreateUVN();
}

PerspectiveCamera::PerspectiveCamera(const Vector3& p, const Vector3& l) {
    m_pos	= p;
    m_look	= l;

    m_up	= Vector3::YVEC();
    if (fabs(m_look.Dot(m_up)) > .90) 
	{
	    m_up = Vector3::ZVEC();
		if (fabs(m_look.Dot(m_up)) > .90) 
		{
			m_up = Vector3::XVEC();
		}
    }

    m_width		= 400;
    m_height	= 400;

    m_fov		= 60.0f;
    m_ar		= 1.0f;

    Init();
    CreateUVN();
}

PerspectiveCamera::PerspectiveCamera(const Vector3& p, const Vector3& l, 
	const Vector3& u) : Camera()  {
    m_pos		= p;
    m_look		= l;
    m_up		= u;

    m_width		= 400;
    m_height	= 400;

    m_fov		= 60.0f;
    m_ar		= 1.0f;

    Init();
    CreateUVN();
}

PerspectiveCamera::~PerspectiveCamera() {
}

void PerspectiveCamera::SetImageDimension(UINT32 width, UINT32 height) {
    m_width = width;
    m_height = height;
    Init();
}

void PerspectiveCamera::SetFov(float fov) {
    m_fov = fov;    
    Init();
}

void PerspectiveCamera::SetAspectRatio(float ar) {
    m_ar = ar;    
    Init();
}

void PerspectiveCamera::Orient(const Vector3& l) {
    m_look = l;
    // Multiple ways to orient.  Pick one that has better numerical
    // stability
    m_up	= Vector3::YVEC();
    if (fabs(m_look.Dot(m_up)) > .90) {
	    m_up = Vector3::ZVEC();
		if (fabs(m_look.Dot(m_up)) > .90) {
			m_up = Vector3::XVEC();
		}
    }
    CreateUVN();
}

void PerspectiveCamera::Orient(const Vector3& l, const Vector3& u) {
    m_look = l;
    m_up = u;
    CreateUVN();
}

void PerspectiveCamera::Translate(const Vector3& t) {
    m_pos += t;
}

void PerspectiveCamera::SetPosition(const Vector3& p) {
    m_pos = p;
}

void PerspectiveCamera::GenerateRay(Ray& ray, float x, float y) {
    DOUBLE du = -au + m_dau*x;
    DOUBLE dv =  av - m_dav*y;

    Vector3 d(du, dv, -1);
    m_basis.TransformToWorld(d);
    ray.Set(m_pos, d);
}

Vector3 PerspectiveCamera::ToCameraSpace(const Vector3& v) const {
    Vector3 ret = v - m_pos;
    m_basis.TransformToObject(ret);	

    DOUBLE depth = 1.0 / (2.0 * tan(DEGREE_TO_RAD(m_fov) / 2.0f)) * m_width;
    DOUBLE rhw = depth / -ret[2];
    ret[0] = m_width*.5 - (-rhw  * ret[0]);
    ret[1] = m_height*.5 - (rhw * ret[1]);
    ret[2] = rhw;

    return ret;
}

void PerspectiveCamera::CreateUVN() {
    Vector3 u, v, n;

    n = -m_look.GetDirection();
	u = n.Cross(m_up);
	u.Normalize();
    v = u.Cross(n);

    m_basis.Set(u, v, n);
}

void PerspectiveCamera::Init() {
    d_width = 2.0 / (m_width - 1.0);
    d_height = 2.0 / (m_height - 1.0);

    au = (FLOAT)tan(DEGREE_TO_RAD(m_fov * .5));
    av = au / m_ar;

    m_dau = d_width * au;
    m_dav = d_height * av;
}

}

