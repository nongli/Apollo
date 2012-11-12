#include "PathTracer.h"
#include "ApolloStructs.h"
#include "SamplerUtil.h"
#include "Shader.h"
#include "Primitive.h"

namespace Apollo {

Color4f PathTracer::RayTrace(const Ray* ray) {
    Color4f result;
    FLOAT weight_per_sample = 1.0 / m_settings.samples_per_pixel;
    for (int i = 0; i < m_settings.samples_per_pixel; ++i) {
        Ray copy(*ray);
        Color4f color = PathTraceInternal(&copy, 0);
        result.MultAggregate(color, weight_per_sample);
    }
    return result;
}

Color4f PathTracer::PathTraceInternal(const Ray* ray, int depth) {
    Color4f color;
    Intersection intersection;
    SurfaceElement surfel;
    BRDF brdf;

    Accel* accel = m_scene->GetAccel();
    if (accel->Intersect(*ray, intersection)) {
		color = Color4f::BLACK();
        surfel.Init(&intersection, ray);
        const Shader* shader = surfel.intersection->primitive->GetShader();
        if (shader) {
            brdf.Reset();
            shader->GetBRDF(surfel, &brdf);
            if (depth < m_settings.max_depth) {
                // Generate new ray randomly in the hemisphere
                // TODO: switch to importance sampled cos-weighted
                Vector3 rand_dir = SamplerUtil::RandHemiSphereSample(surfel.normal);
                Ray next_ray(surfel.iPoint, rand_dir);
                Color4f bounce_color = PathTraceInternal(&next_ray, depth + 1);
                FLOAT weight = rand_dir.Dot(surfel.normal);
                bounce_color *= weight;
                color.MultAggregate(bounce_color, brdf.diffuse);
            } 
            
            Color4f shade_color = shader->Shade(m_scene, surfel, brdf);
            color += shade_color;
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

void PathTracer::Initialize() {
    if (m_settings.max_depth == 0) m_settings.max_depth = 1;
}

PathTracer::~PathTracer() {
    //Empty
}

}
