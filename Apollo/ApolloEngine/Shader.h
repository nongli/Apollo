#pragma once
#include "ApolloCommon.h"
#include "TextureSampler.h"
#include "Scene.h"

/*
 * Shader abstraction.  This can handle both physically based shaders (materials)
 * with BRDFs or less faithful approximation shaders (Phong)
 */
namespace Apollo {
    class BRDF {
    public:
        enum TYPE {
            DIFFUSE,
            SPECULAR,
            REFLECTIVE,
            EMISSIVE
        };

    public: 
        void Reset() {
            memset(this, 0, sizeof(BRDF));
        }

    public:
        Color4f diffuse;
        Color4f specular;
        Color4f reflective;
        Color4f emissive;
    };

    class Shader {
    public:
        virtual Color4f Shade(const Scene* scene, const SurfaceElement&, const BRDF&) const = 0;
        virtual void GetBRDF(const SurfaceElement&, BRDF* brdf) const = 0;
        virtual bool DoesShaderContain(BRDF::TYPE) const = 0;

    // Static functions that are common across multiple kinds of shaders
    public:
        static Color4f ComputeDirectIllumination(const Scene* scene, const SurfaceElement& surfel);

    protected:
        Shader() {}
    };

    // Some default simple shaders
    class LambertianShader : public Shader {
    public:
        LambertianShader(const Color4f& color) : m_color(color) {}

    public:
        Color4f Shade(const Scene* scene, const SurfaceElement& surfel, const BRDF& brdf) const {
            return brdf.diffuse * ComputeDirectIllumination(scene, surfel);
        }

        bool DoesShaderContain(BRDF::TYPE type) const {
            if (type == BRDF::DIFFUSE) return true;
            return false;
        }

        void GetBRDF(const SurfaceElement& surfel, BRDF* brdf) const {
            m_color.GetSample(surfel.uv, brdf->diffuse);
        }

    private:
        TextureSampler<Color4f> m_color;
    };

    inline Color4f Shader::ComputeDirectIllumination(const Scene* scene, const SurfaceElement& surfel) {
        Color4f color = Color4f::BLACK();
        const std::vector<Light*>& lights = scene->GetLights();
        for (UINT i = 0; i < lights.size(); i++) {
            // TODO
            Vector3 L = lights[i]->GetPosition() - surfel.iPoint;
            DOUBLE distance = L.Magnitude();
            L.Normalize();
            FLOAT LN = (FLOAT)L.Dot(surfel.normal);
            
            if (LN > 0) {
                if (lights[i]->GetCastShadow()) {
                    Ray shadowRay(surfel.iPoint, L);
                    shadowRay.t_max = distance;
                    if (scene->GetAccel()->Intersect(shadowRay)) {
                        continue;
                    }
                }

                Color4f lightColor = lights[i]->GetRadiance(Vector3::ZERO(), distance);
                color.MultAggregate(lightColor, LN);
            }
        }
        return color + scene->GetAmbientLight();
    }
}
