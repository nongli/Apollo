#include "Mesh.h"
#include "AABox.h"
#include "Triangle.h"

using namespace std;

namespace Apollo {

Mesh::Mesh() {
}

Mesh::Mesh(const Mesh& s) {
	ApolloException::NotYetImplemented();    
    UNREFERENCED_PARAMETER(s);
}

Mesh::Mesh(const vector<Vector3>& vertices, const vector<Vector3>& normals, 
           const vector<UV>& uvs, const vector<Triangle::Data>& triangles) {
    m_vertices = vertices;
    m_normals = normals;
    m_uvs = uvs;

    for (UINT32 i = 0; i < triangles.size(); ++i) {
        m_triangles.push_back(new Triangle(this, triangles[i]));
    }
}

void Mesh::Init() {
    for (UINT32 i = 0; i < m_triangles.size(); i++) {
		m_triangles[i]->Init();    
    }
}

Mesh::~Mesh() {
    Cleanup();
}

void Mesh::Cleanup() {
    for (UINT32 i = 0; i < m_triangles.size(); i++) {
		delete m_triangles[i];
	}
    m_triangles.clear();
}

void Mesh::GetAABoundingBox(AABox& box) const {
	ApolloException::NotYetImplemented();  
    UNREFERENCED_PARAMETER(box);
}

UINT32 Mesh::GetNumPrimitives() const {
    return m_triangles.size();
}

void Mesh::GetPrimitives(std::vector<const Primitive*>& res) const {
	for (UINT32 i = 0; i < m_triangles.size(); i++) {
		res.push_back(m_triangles[i]);
	}
}

void Mesh::Transform(const OrthonormalBasis& basis) {
	ApolloException::NotYetImplemented();  
    UNREFERENCED_PARAMETER(basis);
}

FLOAT Mesh::SurfaceArea() const {
	FLOAT sa = 0;
	for (UINT32 i = 0; i < m_triangles.size(); i++) {
		sa += m_triangles[i]->SurfaceArea();
	}
	return sa;
}

bool Mesh::Intersect(const Ray& ray, Intersection& in) const {
	ApolloException::NotYetImplemented();
    UNREFERENCED_PARAMETER(ray);
    UNREFERENCED_PARAMETER(in);
    return NO_HIT;
}

bool Mesh::Intersect(const Ray& ray) const {
	ApolloException::NotYetImplemented();
    UNREFERENCED_PARAMETER(ray);
    return NO_HIT;
} 

}
