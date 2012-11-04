#pragma once
#include "ApolloCommon.h"
#include "TextureSampler.h"
#include "Shader.h"
#include "Scene.h"

namespace Apollo {
    class BlinnPhongShader : public Shader {
    public:
        BlinnPhongShader(const Color4f& diffuse, const Color4f& specular, FLOAT shininess) : 
            m_diffuse(diffuse), m_specular(specular), m_shininess(shininess) {
        }

        Color4f Shade(const Scene* scene, const SurfaceElement& surfel) const {
            FLOAT shininess;
            Color4f diffuseColor, specularColor;

            m_diffuse.GetSample(surfel.uv, diffuseColor);
            m_specular.GetSample(surfel.uv, specularColor);
            m_shininess.GetSample(surfel.uv, shininess);

            Color4f specular = Color4f::ZERO();
            Color4f diffuse = Color4f::ZERO();

            const std::vector<Light*>& lights = scene->GetLights();
            for (UINT i = 0; i < lights.size(); i++) {
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
                    
                    Color4f lightColor = lights[i]->GetRadiance(Vector3::ZERO(), 0);

                    if (specularColor.MagnitudeRGB() != 0) {
                        Vector3 V = -surfel.ray->direction;
                        Vector3 H = (L + V) / 2;
                        FLOAT NH = (FLOAT)H.Dot(surfel.normal);
                        if (NH > 0) {
                            FLOAT specularHighlight = powf(NH, shininess);
                            specular.MultAggregate(lightColor, specularHighlight);
                        }
                    }
                    diffuse.MultAggregate(lightColor, LN);
                }
            }

            return diffuseColor*diffuse + specularColor*specular;
        }

        bool DoesShaderContain(BRDF::TYPE type) const {
            if (type == BRDF::DIFFUSE || type == BRDF::REFLECTIVE || type == BRDF::SPECULAR) return true;
            return false;
        }

    private:
        TextureSampler<Color4f> m_diffuse;
        TextureSampler<Color4f> m_specular;
        TextureSampler<FLOAT> m_shininess;
    };
}
