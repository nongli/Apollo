#pragma once

#include "ApolloCommon.h"
#include "ApolloStructs.h"
#include "Primitive.h"

namespace Apollo {

void SurfaceElement::Init(const Intersection* i, const Ray* r) {
	intersection	= i;
	ray				= r;
	iPoint			= r->origin + i->t*r->direction;
	i->primitive->GetNormalAndUV(*this);
}

}
