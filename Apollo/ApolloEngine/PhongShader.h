#pragma once
#include "ApolloCommon.h"
#include "TextureSampler.h"
#include "Shader.h"
#include "Scene.h"

namespace Apollo {
    class PhongShader : public Shader {
    public:
        PhongShader(const Color4f& diffuse, const Color4f& specular, FLOAT shininess) : 
            m_diffuse(diffuse), m_specular(specular), m_shininess(shininess) {
        }

        void GetBRDF(const SurfaceElement& surfel, BRDF* brdf) const {
            m_diffuse.GetSample(surfel.uv, brdf->diffuse);
            m_specular.GetSample(surfel.uv, brdf->specular);
            brdf->reflective = brdf->specular;
        }

        Color4f Shade(const Scene* scene, const SurfaceElement& surfel, const BRDF& brdf) const {
            FLOAT shininess;
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

                    if (brdf.specular.MagnitudeRGB() != 0) {
                        Vector3 R = L.ReflectVector(surfel.normal);
                        Vector3 V = -surfel.ray->direction;
                        R.Normalize();
                        FLOAT RV = (FLOAT)R.Dot(V);
                        if (RV > 0) {
                            FLOAT specularHighlight = powf(RV, shininess);
                            specular.MultAggregate(lightColor, specularHighlight);
                        }
                    }
                    diffuse.MultAggregate(lightColor, LN);
                }
            }

            return brdf.diffuse*diffuse + brdf.specular*specular;
        }

        bool DoesShaderContain(BRDF::TYPE type) const {
            if (type == BRDF::DIFFUSE || type == BRDF::REFLECTIVE || type == BRDF::SPECULAR) {
                return true;
            }
            return false;
        }

    private:
        TextureSampler<Color4f> m_diffuse;
        TextureSampler<Color4f> m_specular;
        TextureSampler<FLOAT> m_shininess;
    };
}
