/**
 * This is the light class.  It provides the functionality for
 * basic light types but can be subclassed for more sophisticated light
 * models.  The brdf also has an emissive component so meshes could be
 * be used as light sources.
 */

#pragma once

#include "ApolloCommon.h"

namespace Apollo {
    class Light {
    public:
        enum LIGHTTYPE {
            AMBIENT,
	        POINT,
            DIRECTIONAL,
	        SPOT,
	        PROJECTION,
        };

        enum FALLOFF {
	        LIGHT_FALLOFF_NONE	= 0,
	        LIGHT_FALLOFF_LINEAR,
	        LIGHT_FALLOFF_QUADRATIC,
        };

    public:
		static Light* CreatePointLight(const Vector3& pos, const Color4f& color, FLOAT intensity);

        Light(Light::LIGHTTYPE type);

        LIGHTTYPE GetType() const;

        // Set/Get Falloff
	    void SetLightFalloff(FALLOFF);
	    FALLOFF GetLightFalloff() const;

        // Set/Get Position
	    void SetPosition(const Vector3& pos);
	    const Vector3& GetPosition() const;

        // Sets/Gets color
	    void SetColor(const Color4f& color);
	    const Color4f& GetColor() const;

        // Sets/Gets intensity of light
	    void SetIntensity(FLOAT intensity);
	    const FLOAT GetIntensity() const;

	    // Sets/Gets direction of light
	    void SetDirection(const Vector3& dir);
	    const Vector3& GetDirection() const;

        // Sets/Gets if the light casts shadows
	    void SetCastShadow(bool shadow);
	    bool GetCastShadow() const;

	    // Returns the unobstructed radiance from this light 
	    // dir should be the direction to the light
	    virtual Color4f GetRadiance(const Vector3& dir, double distance);

	    virtual ~Light() { /* Empty */ };

	protected:
        LIGHTTYPE   m_type;

	    Vector3		m_position;
	    Vector3		m_direction;
	    bool		m_shadow;
	    
	    Color4f		m_color;
	    FLOAT		m_intensity;

	    FALLOFF	    m_fallOff;

	    /* Spot light data */
	    double		spotCosMaxAngle;
	    double		spotCosFallOffAngle;
	    double		spotAngle;
	    double		spotPenumbraAngle;
	    double		spotInvAngle;
    };

    inline Color4f Light::GetRadiance(const Vector3& dir, double d) {
        UNREFERENCED_PARAMETER(dir);
        UNREFERENCED_PARAMETER(d);

        //TODO
	    switch (m_type) {
            case AMBIENT:
	        case POINT:
                return m_color * m_intensity;
		        break;

	        default:
                ApolloException::NotYetImplemented();
	    }
        return Color4f::BLACK();
    }

    inline Light::Light(LIGHTTYPE type) : 
        m_color(Color4f::WHITE()),
		m_intensity(1),
        m_type(type),
        m_shadow(true),
        m_fallOff(LIGHT_FALLOFF_QUADRATIC) {
    }
    
    inline void Light::SetLightFalloff(FALLOFF type) {
	    m_fallOff = type;
    }

    inline Light::FALLOFF Light::GetLightFalloff() const {
	    return m_fallOff;
    }

    inline Light::LIGHTTYPE Light::GetType() const {
	    return m_type;
    }

    inline void Light::SetPosition(const Vector3& position) {
	    m_position = position;
    }

    inline const Vector3& Light::GetPosition() const {
	    return m_position;
    }

    inline void Light::SetColor(const Color4f& color) {
	    m_color = color;
    }

    inline const Color4f& Light::GetColor() const {
	    return m_color;
    }

    inline void Light::SetIntensity(FLOAT intensity) {
	    m_intensity = intensity;
    }

    inline const FLOAT Light::GetIntensity() const {
	    return m_intensity;
    }

    inline void Light::SetDirection(const Vector3& direction) {
	    m_direction = direction;
	    m_direction.Normalize();
    }

    inline const Vector3& Light::GetDirection() const {
	    return m_direction;
    }

    inline void Light::SetCastShadow(bool shadow) {
	    m_shadow = shadow;
    }

    inline bool Light::GetCastShadow() const {
	    return m_shadow;
    }
}
