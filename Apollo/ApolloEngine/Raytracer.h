/*
 * Ray tracer rendering algorithms.  Theses are "pixel based" (as opposed to say mlt)
 */

#pragma once

#include "ApolloCommon.h"
#include "Renderer.h"
#include "PrimarySampler.h"

namespace Apollo {
    class RayTracer : public Renderer {
    // Subclass should override
    protected:
        virtual Color4f RayTrace(const Ray* ray) = 0;
        virtual void Initialize() = 0;

    public:
        virtual const Image* Render(const Scene* scene, const Rect<UINT>& region);

    public:
        // Render Settings
        void SetPrimarySampler(PrimarySampler*);
        void SetRenderBucket(RenderBucket*);

    public:
        ~RayTracer();

    protected:
        void InitializeInternal();

        RayTracer(const std::string& name);

        PrimarySampler* m_primarySampler;
        RenderBucket* m_renderBucket;
        const Scene* m_scene;
    };
}
