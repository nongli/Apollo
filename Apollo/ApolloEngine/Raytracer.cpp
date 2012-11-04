#include "Raytracer.h"
#include "ScanLineBucket.h"

namespace Apollo {

RayTracer::RayTracer(const std::string& name) :
    Renderer(name),
    m_primarySampler(nullptr),
    m_renderBucket(nullptr),
    m_scene(nullptr) {
}

const Image* RayTracer::Render(const Scene* scene, const Rect<UINT>& region) {
    assert (scene != nullptr);
    m_scene = scene;
	m_env_shader = scene->GetEnvironmentShader();

    InitializeInternal();
    Initialize();

    Camera* camera = scene->GetCamera();
    m_primarySampler->SetRenderBucket(m_renderBucket);
    m_primarySampler->Reset(region.y, region.x, region.y + region.h, region.x + region.w);

    Image* image = new Image(camera->GetImageWidth(), camera->GetImageHeight());

    Ray ray;
    PrimarySample sample(image);
    Color4f color;

    while (m_primarySampler->GetNextSample(sample)) {
        camera->GenerateRay(ray, sample.xFilm, sample.yFilm);
        color = RayTrace(&ray);
        sample.SetColor(color);
    }

    return image;
}

void RayTracer::SetPrimarySampler(PrimarySampler* sampler) {
    assert(m_primarySampler == nullptr);
    m_primarySampler = sampler;
}

void RayTracer::SetRenderBucket(RenderBucket* bucket) {
    assert(m_renderBucket == nullptr);
    m_renderBucket = bucket;
}

void RayTracer::InitializeInternal() {
    Camera* camera = m_scene->GetCamera();
    if (m_primarySampler == nullptr) {
        m_primarySampler = new PrimarySampler;
    }
    if (m_renderBucket == nullptr) {
        m_renderBucket = new ScanLineBucket(camera->GetImageWidth(), camera->GetImageHeight());
    }
}

RayTracer::~RayTracer() {
    //TODO - scoped pointers?  Allocator pool?
}

}
