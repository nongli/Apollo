/*
 * Ray tracer rendering algorithms.  This are "pixel based" (as opposed to path based)
 */

#pragma once

#include "ApolloCommon.h"
#include "RayTracer.h"

namespace Apollo {
    class WhittedRayTracer : public RayTracer {
    public:
        WhittedRayTracer() : RayTracer("Whitted Ray Tracing") {}

    public:
        ~WhittedRayTracer();

    protected:
        virtual Color4f RayTrace(const Ray* ray);
        virtual void Initialize();
    };
}
