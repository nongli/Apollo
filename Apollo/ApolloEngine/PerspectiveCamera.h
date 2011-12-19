/**
 * Default camera 
 */

#pragma once

#include "ApolloCommon.h"
#include "Camera.h"

namespace Apollo {

    class PerspectiveCamera : public Camera {
	public:
	    // Creates camera with reasonable defaults
	    PerspectiveCamera();
	    PerspectiveCamera(const Vector3& pos, const Vector3& look);
	    PerspectiveCamera(const Vector3& pos, 
						  const Vector3& look, 
			              const Vector3& up);

	    virtual ~PerspectiveCamera();

	    virtual void SetImageDimension(UINT32 width, UINT32 height);
	    virtual void SetFov(FLOAT fov);
	    virtual void SetAspectRatio(FLOAT ar);

        virtual UINT32 GetImageWidth() const { return m_width; }
        virtual UINT32 GetImageHeight() const { return m_height; }
        virtual FLOAT GetFov() const { return m_fov; }
        virtual FLOAT GetAspectRatio() const { return m_ar; }

	    virtual void Orient(const Vector3& look);
	    virtual void Orient(const Vector3& look, const Vector3& up);

	    // Fills in the ray with the information in world
	    // coordinates.  The parameters x and y are coordinates
	    // in the film plane 
	    virtual void GenerateRay(Ray& ray, FLOAT x, FLOAT y);

	    virtual void Translate(const Vector3& t);

	    virtual void SetPosition(const Vector3& p);

	    // Given a point in world space, return the point in camera
	    // object space 
	    virtual Vector3 ToCameraSpace(const Vector3& v) const;

	protected:
	    virtual void CreateUVN();
	    virtual void Init();

	    OrthonormalBasis	m_basis;
	    Vector3				m_pos, m_look, m_up;
	    UINT32				m_width, m_height;
	    DOUBLE				d_width, d_height;

        DOUBLE              m_dau, m_dav;

	    FLOAT				m_fov, m_ar;
	    FLOAT				au, av;
    };
};


