/*
 * Primitives are the base unit of intersection.  Models are composed
 * of multiple primitives.  Breaking up a primitive does not make sense.
 */

#pragma once

#include "ApolloCommon.h"
#include "ApolloStructs.h"
#include "Model.h"

namespace Apollo {
    class Model;
    class AABox;
    class Shader;

    class Primitive {
	public:
        Primitive(Model* model) : m_model(model) {}
	    
	    virtual void Init() = 0;

        virtual ~Primitive() {}

	    // Returns the shader on this primitive
        const Shader* GetShader() const { return m_model->GetShader(); }

	    // Returns axis aligned bounding box for primitive
	    virtual void GetAABoundingBox(AABox&) const = 0;
	    
	    // Returns the model that this primitive is part of
        virtual Model* GetParentModel() const { return m_model; }

	    // Intersection methods
	    virtual bool Intersect(const Ray&, Intersection&) const = 0;
	    virtual bool Intersect(const Ray&) const = 0;

	    // Fills in the SurfaceElement with the world space normal and
		// UV texture coordinate for intersection point
	    virtual void GetNormalAndUV(SurfaceElement&) const = 0;

	    // Returns the surface area of the primitive
	    virtual float SurfaceArea() const = 0;

	protected:
	    Model* m_model;
    };
}
