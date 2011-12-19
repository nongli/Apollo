#include "Primitive.h"
#include "Model.h"
#include "Sphere.h"
#include "AABox.h"

namespace Apollo {

//
// Sphere
//
void Sphere::Init() {
	Primitive::m_model = this;
}

void Sphere::GetPrimitives(std::vector<const Primitive*>& res) const { 
	res.push_back(this);
};

void Sphere::GetAABoundingBox(AABox& box) const {
	Vector3 rad = Vector3(radius, radius, radius);
	box.Add(center - rad);
	box.Add(center + rad);
}

void Sphere::Transform(const OrthonormalBasis&) {
	Apollo::ApolloException::NotYetImplemented();
}

//
//  AABox
//
void AABox::Init() {
	Primitive::m_model = this;
}

void AABox::GetPrimitives(std::vector<const Primitive*>& res) const { 
	res.push_back(this);  
};

void AABox::GetAABoundingBox(AABox& box) const {
	box.Add(Vector3(min));
	box.Add(Vector3(max));
}

void AABox::Transform(const OrthonormalBasis&) {
	Apollo::ApolloException::NotYetImplemented();
}

//
//  InfinitePlane
//
void InfinitePlane::Init() {
	Primitive::m_model = this;
}

void InfinitePlane::GetPrimitives(std::vector<const Primitive*>& res) const { 
	res.push_back(this); 
}

void InfinitePlane::GetAABoundingBox(AABox& box) const {
	box.Add(Vector3(-INFINITY, -INFINITY, -INFINITY));
	box.Add(Vector3(INFINITY, INFINITY, INFINITY));
}

void InfinitePlane::Transform(const OrthonormalBasis&) {
	Apollo::ApolloException::NotYetImplemented();
}

}
