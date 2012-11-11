/*
 * Ray tracer rendering algorithms.  This are "pixel based" (as opposed to path based)
 */

#pragma once

#include "ApolloCommon.h"
#include "RayTracer.h"

namespace Apollo {
    class WhittedRayTracer : public RayTracer {
    public:
        struct Settings : RayTracer::Settings {
            int reflection_depth;

            Settings() {
                reflection_depth = 0;
            }
        };

        WhittedRayTracer(const Settings& settings) : 
            RayTracer("Whitted Ray Tracing"),
            m_settings(settings) {}

    public:
        ~WhittedRayTracer();

    protected:
        virtual Color4f RayTrace(const Ray* ray);
        virtual void Initialize();
        Color4f RayTraceInternal(const Ray* ray, int depth);

        Settings m_settings;
    };
}
