#pragma once

#include <math.h>
#include <float.h>

#include "Types.h"
#include "Constants.h"
#include "Primes.h"
#include "Rand.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MAX3(a, b, c) \
	( (a) > (b) ? (a) > (c) ? (a) : (c) \
	: (b) > (c) ? (b) : (c) )

#define MIN3(a, b, c) \
	( (a) < (b) ? (a) < (c) ? (a) : (c) \
	: (b) < (c) ? (b) : (c) )

#define CLAMP(x, min, max) ((x) < (min) ? (min) : (x) > (max) ? (max) : (x))

#define EQ(x, y) (fabs((x) - (y)) < EPSILON)
#define NEQ(x, y) (fabs((x) - (y)) > EPSILON)

#define RAD_TO_DEGREE(a) ((a) * APOLLO_PI_INV * 180.0)
#define DEGREE_TO_RAD(a) ((1/180.0) * (a) * APOLLO_PI)

#define IS_IN_INTERVAL(a, b, c) ((((a) >= (b)) && ((a) <= (c))))

namespace Apollo
{
    template <typename T>
    struct Rect
    {
        T x, y, w, h;

        Rect()
        {
            x = y = w = h = 0;
        }

        Rect(T x, T y, T w, T h)
        {
            this->x = x;
            this->y = y;
            this->w = w;
            this->h = h;
        }
    };
};