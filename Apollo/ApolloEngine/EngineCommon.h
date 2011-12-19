#pragma once

#include "ApolloCommon.h"
#include "Image.h"

typedef INT64	OBJECT_ID;

#define HIT		1
#define NO_HIT	0

namespace Apollo {
	OBJECT_ID GetLUID();

    struct UV {
		FLOAT u;
		FLOAT v;
		UV() { u = v = 0;};
		UV(FLOAT u_, FLOAT v_) { u = u_; v= v_;};
    };
};

#include "OrthonormalBasis.h"
