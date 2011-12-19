#pragma once
/**
 * This is model for a plane made from a mesh of 2 triangles
 */


#include "Mesh.h"
#include "Triangle.h"

namespace Apollo {
    class Plane : public Mesh {
	public:
	    Plane();
	    //
	    // Construct a plane passing it the four vertices in CCW order
	    // It is assumed that the points are coplanar.
	    // They will be uv mapped:
	    //		 A  B
	    //		 D  C
	    //	A: (0, 0)
	    //	B: (1, 0)
	    //	C: (1, 1)
	    //	D: (0, 1)
	    //
	    Plane(const Vector3 v[4]);

	    virtual ~Plane() { /*Empty */ };

	    virtual bool Intersect(const Ray&, Intersection&) const;
	    virtual bool Intersect(const Ray&) const;
    };

    inline Plane::Plane(const Vector3 v[4]) {
		m_vertices.push_back(v[0]);		
		m_vertices.push_back(v[1]);		
		m_vertices.push_back(v[2]);		
		m_vertices.push_back(v[3]);		

		Vector3 v21 = v[1] - v[2];
		Vector3 v23 = v[3] - v[2];

		Vector3 normal = v23.Cross(v21).GetDirection();
		m_normals.push_back(normal);

		m_uvs.push_back(UV(0, 0));
		m_uvs.push_back(UV(1, 0));
		m_uvs.push_back(UV(1, 1));
		m_uvs.push_back(UV(0, 1));

		Triangle* t1 = new Triangle(this);
		Triangle* t2 = new Triangle(this);
		
		t1->SetVertices(0, 1, 2);
		t1->SetNormals(0, 0, 0);
		t1->SetUVs(0, 1, 2);

		t2->SetVertices(0, 2, 3);
		t2->SetNormals(0, 0, 0);
		t2->SetUVs(0, 2, 3);

		m_triangles.push_back(t1);
		m_triangles.push_back(t2);
    }

    inline bool Plane::Intersect(const Ray& ray, Intersection& in) const {
		bool hit = false;
		hit |= m_triangles[0]->Intersect(ray, in);
		hit |= m_triangles[1]->Intersect(ray, in);
		return hit;
    }

    inline bool Plane::Intersect(const Ray& ray) const {
		bool hit = false;
		hit |= m_triangles[0]->Intersect(ray);
		hit |= m_triangles[1]->Intersect(ray);
		return hit;
    }    
}
