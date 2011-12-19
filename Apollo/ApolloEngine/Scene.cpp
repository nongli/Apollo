#include "Scene.h"
#include "LinearAccel.h"

using namespace std;

namespace Apollo {

Scene::Scene(const string& name) : m_name(name),
    m_accel(nullptr),
    m_camera(nullptr),
    m_lightDirty(false),
    m_cameraDirty(false),
    m_geometryDirty(true),
    m_ambientLight(Color4f::BLACK()) {
}

Scene::~Scene() {
    //TODO
}

void Scene::AddLight(Light* light) {
    if (light->GetType() == Light::AMBIENT) {
        m_ambientLight.MultAggregate(light->GetColor(), light->GetIntensity());
        return;
    }
    m_lights.push_back(light);
    m_lightDirty = true;
}

void Scene::Initialize() {
    assert (m_camera != nullptr);
    if (!m_lightDirty && !m_geometryDirty && !m_cameraDirty) return;

    if (m_accel == nullptr) {
        m_accel = new LinearAccel;
    }

    for (UINT i = 0; i < m_models.size(); i++) {
        m_models[i]->Init();
        m_accel->AddGeometry(m_models[i]);
    }

    m_accel->Init();
    m_lightDirty = false;
    m_geometryDirty = false;
}

}
