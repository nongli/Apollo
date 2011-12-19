#include "LinearAccel.h"
#include "Model.h"
#include "Primitive.h"

using namespace std;

namespace Apollo {

LinearAccel::LinearAccel() {
    m_primitives.clear();
    m_models.clear();
}

LinearAccel::~LinearAccel() {
	m_models.clear();
	m_primitives.clear();
}

void LinearAccel::SetGeometry(vector<Model*>& models) {
	m_models.clear();
	for (UINT32 i = 0; i < models.size(); i++) {
		AddGeometry(models[i]);
	}
}

void LinearAccel::AddGeometry(Model* model) {
	m_models.push_back(model);
}

bool LinearAccel::Init() {
    return Rebuild();
}

bool LinearAccel::Rebuild() {
    m_primitives.clear();

    for (UINT32 m = 0; m < m_models.size(); m++) {
		m_models[m]->GetPrimitives(m_primitives);
	}

    return true;
}

bool LinearAccel::Intersect(const Ray& ray, Intersection& in) const {
    bool hit = false;
    for (UINT32 n = 0; n < m_primitives.size(); n++) {
		const Primitive* prim = m_primitives[n];
		hit |= prim->Intersect(ray, in);
    }
    return hit ? HIT : NO_HIT;
}

bool LinearAccel::Intersect(const Ray& ray) const {
    for (UINT32 n = 0; n < m_primitives.size(); n++) {
		if (m_primitives[n]->Intersect(ray)) {
			return HIT;
		}
    }
    return NO_HIT;
}

bool LinearAccel::Intersect(const Ray& ray, std::vector<Intersection>& in) const {
	bool hit = false;
	Intersection inter;

    for (UINT32 n = 0; n < m_primitives.size(); n++) {
		if (m_primitives[n]->Intersect(ray, inter)) {
			hit = true;
			in.push_back(inter);
		}
    }
    return hit;
}

}

