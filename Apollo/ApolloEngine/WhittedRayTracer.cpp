#include "WhittedRaytracer.h"
#include "ApolloStructs.h"
#include "Shader.h"
#include "Primitive.h"

namespace Apollo {

Color4f WhittedRayTracer::RayTrace(const Ray* ray) {
    return RayTraceInternal(ray, 0);
}

Color4f WhittedRayTracer::RayTraceInternal(const Ray* ray, int depth) {
    Color4f color;
    Intersection intersection;
    SurfaceElement surfel;
    BRDF brdf;

    Accel* accel = m_scene->GetAccel();
    if (accel->Intersect(*ray, intersection)) {
		color = Color4f::RED();
        surfel.Init(&intersection, ray);
        const Shader* shader = surfel.intersection->primitive->GetShader();
        if (shader) {
            brdf.Reset();
            shader->GetBRDF(surfel, &brdf);
            color = shader->Shade(m_scene, surfel, brdf);

            if (depth < m_settings.reflection_depth && brdf.reflective.MagnitudeRGB() > 0) {
                // TODO handle recursion
            }
        } else {
            color = Color4f(1, 0, 1);
        }
        color.a = 1;        // If we hit somthing, alpha = 1
    } else if (m_env_shader != nullptr) {
		// TODO: this needs to be environment uv coordinates
		color = m_env_shader->Shade(m_scene, surfel, brdf);
    } else {
		color = Color4f::ZERO();
	}
    return color;
}

void WhittedRayTracer::Initialize() {
    //Empty
}

WhittedRayTracer::~WhittedRayTracer() {
    //Empty
}

}
