/*
 * Model class is an object in the scene with the same material and shading
 * properties.  It is composed of 1 or many primitives.
 */

#pragma once

#include "ApolloCommon.h"
#include "ApolloStructs.h"

namespace Apollo {
    class Primitive;
    class AABox;
    class BoundingBox;
    class Shader;
    class OrthonormalBasis;

    class Model {
	public:
	    virtual void Init() = 0;

	    virtual ~Model();

	    // Copy constructor - Implement instancing of models 
	    Model(const Model&);

	    // Returns bounding box around model
	    virtual void GetAABoundingBox(AABox& box) const = 0;
	    //virtual void GetBoundingBox(BoundingBox&) const = 0;

	    // Sets the bounding box for this model
	    //virtual void SetBoundingBox(const BoundingBox&) = 0;

	    // Fills the vector with the primitives making up this model
		virtual void GetPrimitives(std::vector<const Primitive*>&) const = 0;
	    virtual UINT32 GetNumPrimitives() const = 0;

	    // Sets and returns shader for model
	    virtual void SetShader(Shader*);
	    virtual const Shader* GetShader() const;
	   
	    // Transform model and all primitives in model
	    virtual void Transform(const OrthonormalBasis& basis) = 0;

	    // Intersection methods
	    virtual bool Intersect(const Ray&, Intersection&) const = 0;
	    virtual bool Intersect(const Ray&) const = 0;

	    /* Returns surface area of model */
	    virtual FLOAT SurfaceArea() const = 0;

	protected:
	    Model();
	    Shader*				m_shader;
		OrthonormalBasis	m_transform;
    };

    inline Model::Model() {
		m_shader = nullptr;
    }

    inline Model::~Model() {
    }

    inline void Model::SetShader(Shader* s) {
		m_shader = s;
    }

    inline const Shader* Model::GetShader() const {
		return m_shader;
    }
}
