/*
 * Path tracer.  This are "pixel based" (as opposed to path based)
 */

#pragma once

#include "ApolloCommon.h"
#include "RayTracer.h"

namespace Apollo {
    class PathTracer : public RayTracer {
    public:
        struct Settings : RayTracer::Settings {
            int max_depth;
            int samples_per_pixel;

            Settings() {
                max_depth = 1;
                samples_per_pixel = 1;
            }
        };

        PathTracer(const Settings& settings) : 
            RayTracer("Whitted Ray Tracing"),
            m_settings(settings) {}

    public:
        ~PathTracer();

    protected:
        virtual Color4f RayTrace(const Ray* ray);
        Color4f PathTraceInternal(const Ray* ray, int depth);
        virtual void Initialize();

        Settings m_settings;
    };
}
