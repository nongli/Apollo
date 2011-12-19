#pragma once

/**
 * Interface for acceleration data structures for ray tracing 
 */

#include "ApolloCommon.h"
#include "ApolloStructs.h"

namespace Apollo {

    class Model;
	class Primitive;

    class Accel {
	public:
		Accel() {};

	    // Set the geometry for the scene
		virtual void SetGeometry(std::vector<Model*>& models) = 0;
		virtual void AddGeometry(Model* model) = 0;

	    // Perform all prepocessing and build the data structure.
	    // This method must be called before any of the other
	    // methods in this class can be used */
	    virtual bool Init() = 0;

	    // Cleans up the data structre
		virtual ~Accel() {}

	    // Rebuild the data structure.  The application will 
	    // invoke this method if the data structure needs to be
	    // rebuilt (ie: geometry changes).  The data structure
	    // should delta process if possible
	    virtual bool Rebuild() = 0;
	    
	    // Intersection for primary rays. - Returns the first
	    // intersection. 
	    virtual bool Intersect(const Ray&, Intersection&) const = 0;

	    // Intersection for shadow rays  - Returns if there is any
	    // intersection.
	    virtual bool Intersect(const Ray&) const = 0;

	    // Intersection for certain global illumation algorithms -
	    // returns all intersections
		virtual bool Intersect(const Ray&, std::vector<Intersection>&) const = 0;
    };
};
