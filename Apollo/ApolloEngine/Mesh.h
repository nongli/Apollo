#pragma once

/**
 * This is model for a generic triangular mesh
 */
#include "Model.h"
#include "Triangle.h"

namespace Apollo {
    class Primitive;
    class Triangle;
    class Mesh : public Model {
	
	public:
	    Mesh();
	    Mesh(const Mesh& s);
        Mesh(const std::vector<Vector3>& vertices, const std::vector<Vector3>& normals, 
             const std::vector<UV>& uvs, const std::vector<Triangle::Data>& triangles);

	    virtual void Init();

	    virtual ~Mesh();

	    virtual bool Intersect(const Ray&, Intersection&) const;
	    virtual bool Intersect(const Ray&) const;

	    virtual void GetAABoundingBox(AABox&) const;
	   
	    virtual UINT32 GetNumPrimitives() const;
	    virtual void GetPrimitives(std::vector<const Primitive*>&) const;
	    
	    virtual void Transform(const OrthonormalBasis& basis);

	    virtual float SurfaceArea() const;

	protected:
        friend class Triangle;

		std::vector<Vector3>	    m_vertices;
		std::vector<Vector3>	    m_normals;
		std::vector<UV>				m_uvs;
		std::vector<Triangle*>		m_triangles;
	    OrthonormalBasis			m_transform;

	protected:
	    void Cleanup();
    };
}
