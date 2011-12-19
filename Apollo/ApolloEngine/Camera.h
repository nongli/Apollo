/**
 * This is an interface for a camera
 */
#pragma once

#include "EngineCommon.h"
#include "ApolloStructs.h"

namespace Apollo {

    class Camera {
	public:
	    void SetID(OBJECT_ID id);
	    OBJECT_ID GetID() const;

	    virtual void SetImageDimension(UINT32 width, UINT32 height) = 0;

	    virtual void SetFov(FLOAT fov) = 0;
	    virtual void SetAspectRatio(FLOAT ar) = 0;
        virtual void Orient(const Vector3& look) = 0;
	    virtual void Orient(const Vector3& look, const Vector3& up) = 0;

	    virtual UINT32 GetImageWidth() const = 0;
	    virtual UINT32 GetImageHeight() const = 0;
	    virtual FLOAT GetFov() const = 0;
	    virtual FLOAT GetAspectRatio() const = 0;

	    // Fills in the ray with the information in world
	    // coordinates.  The parameters x and y are coordinates
	    // in the film plane 
	    virtual void GenerateRay(Ray& ray, FLOAT x, FLOAT y) = 0;

	    // These happen in world space 
	    virtual void Translate(const Vector3& t) = 0;
	    virtual void SetPosition(const Vector3& p) = 0;

	    // Given a point in world space, return the point in camera
	    // object space
	    virtual Vector3 ToCameraSpace(const Vector3& v) const = 0;

	    /* Clean up camera data */
	    virtual ~Camera() { /* Empty */ };

	protected:
	    Camera() {};

	private:
	    OBJECT_ID m_id;
    };

    inline void Camera::SetID(OBJECT_ID id) {
		m_id = id;
    }

    inline OBJECT_ID Camera::GetID() const {
		return m_id;
    }
		
};

