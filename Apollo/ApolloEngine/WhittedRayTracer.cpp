#include "WhittedRaytracer.h"
#include "ApolloStructs.h"
#include "Shader.h"
#include "Primitive.h"

namespace Apollo {

Color4f WhittedRayTracer::RayTrace(const Ray* ray) {
    Color4f color;
    Intersection intersection;
    SurfaceElement surfel;

    Accel* accel = m_scene->GetAccel();
    if (accel->Intersect(*ray, intersection)) {
        surfel.Init(&intersection, ray);
        const Shader* shader = surfel.intersection->primitive->GetShader();
        if (shader) {
            color = shader->Shade(m_scene, surfel);
        } else {
            color = Color4f(1, 0, 1);
        }
        color.a = 1;        // If we hit somthing, alpha = 1
    } else if (m_env_shader != nullptr) {
		// TODO: this needs to be environment uv coordinates
		color = m_env_shader->Shade(m_scene, surfel);
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
