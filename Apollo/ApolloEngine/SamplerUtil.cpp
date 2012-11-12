#include "SamplerUtil.h"

namespace Apollo {

void SamplerUtil::Init(bool randSeed) {
	if (randSeed) {
		ApolloException::NotYetImplemented();
	} else {
		Apollo::RandSeed(0);
	}
}



Vector3 SamplerUtil::RandHemiSphereSample(const Vector3& n) {
    DOUBLE x, y, z;
    do {
		x = RandDouble() - .5;
		y = RandDouble() - .5;
		z = RandDouble() - .5;
    } while (x*x + y*y + z*z > 1.0 || n[0]*x + n[1]*y + n[2]*z < 0.0);
    return Vector3(x, y, z).GetDirection();
}

Vector3 SamplerUtil::RandHemiSphereCosSample(const Vector3& n, DOUBLE u, DOUBLE v) {
    // Angle from normal
    DOUBLE theta = acos(sqrt(u));

    // Angle about normal
    DOUBLE phi   = APOLLO_2_PI * v;

    // Make a coordinate system
    Vector3 U = n.GetDirection();
    Vector3 V = Vector3::XVEC();
    if (fabs(U.Dot(V)) > .9) V = Vector3::YVEC();

    Vector3 W = U.Cross(V).GetDirection();
    V = W.Cross(U);

    // Convert to rectangular form
    return cos(theta)*U + sin(theta) * (cos(phi)*V + sin(phi)*W);
}

Vector3 SamplerUtil::RandHemiSphereSample(const Vector3& normal, DOUBLE u, DOUBLE v) {
    DOUBLE z	= u;
    DOUBLE r	= sqrt(MAX(0, 1 - z*z));
    DOUBLE phi	= APOLLO_2_PI * v;
    DOUBLE x	= r * cos(phi);
    DOUBLE y	= r * sin(phi);
    if (normal.x*x + normal.y*y + normal.z*z < 0) {
		return -(Vector3(x, y, z).GetDirection());
    } else {
		return (Vector3(x, y, z).GetDirection());
    }
}

void SamplerUtil::RandDiskSample(DOUBLE u, DOUBLE v, DOUBLE& x, DOUBLE& y) {
    DOUBLE r		= sqrt(u);
    DOUBLE theta	= APOLLO_2_PI * v;
    x				= r*cos(theta);
    y				= r*sin(theta);
}

void SamplerUtil::RandConcentricDiskSample(DOUBLE u, DOUBLE v, DOUBLE& x, DOUBLE& y) {
    DOUBLE r, theta;
    DOUBLE sx = 2*u - 1;
    DOUBLE sy = 2*v - 1;

    if (sx >= -sy) {
		if (sx > sy) {
			r = sx;
			if (sy > 0.0) theta = sy/r;
			else theta = 8.0 + sy/r;
		} else {
			r = sy;
			theta = 2.0f - sx/r;
		}
    } else {
		if (sx <= sy) {
			r = -sx;
			theta = 4.0f - sy/r;
		} else {
			r = -sy;
			theta = 6.0f + sx/r;
		}
    }
    theta *= APOLLO_PI * .25;
    x = r*cos(theta);
    y = r*sin(theta);
}

void SamplerUtil::RandomSamples(DOUBLE* samples, UINT32 n) {
    for (UINT32 i = 0; i < n; ++i) {
		*(samples++) = RandDouble();
    }
}

void SamplerUtil::GridSamples(DOUBLE* samples, UINT32 x, UINT32 y) {
    DOUBLE dx = 1.0f / x;
    DOUBLE dy = 1.0f / y;
    DOUBLE ox = -.5f / x;
    DOUBLE oy = -.5f / y;

    for (UINT32 i = 0; i < x; ++i) {
		for (UINT32 j = 0; j < y; ++j) {
			*(samples++) = dx*i + ox;
			*(samples++) = dy*j + oy;
		}
    }
}

void SamplerUtil::JitteredSamples(DOUBLE* samples, UINT32 x, UINT32 y) {
    DOUBLE dx = 1.0f / x;
    DOUBLE dy = 1.0f / y;

    for (UINT32 i = 0; i < x; ++i) {
		for (UINT32 j = 0; j < y; ++j) {
			*(samples++) = dx*i + dx*RandDouble();
			*(samples++) = dy*j + dy*RandDouble();
		}
	}
}

void SamplerUtil::MultiJitteredSamples(DOUBLE* samples, UINT32 x, UINT32 y) {
    DOUBLE subCellWidth = 1.0f / (y*x);

    for (UINT32 i = 0; i < y; ++i) {
		for (UINT32 j = 0; j < x; ++j) {
		   *(samples++) = i*x*subCellWidth + j*subCellWidth + RandDouble()*subCellWidth;
			*(samples++) = j*y*subCellWidth + i*subCellWidth + RandDouble()*subCellWidth;
		}
	}
}

void SamplerUtil::LHCSamples(DOUBLE* samples, UINT32 n, UINT32 dim) {
    DOUBLE delta = 1.f / n;

    for (UINT32 i = 0; i < n; ++i) {
		for (UINT32 j = 0; j < dim; ++j) {
		    samples[dim*i+j] = (i+RandDouble()) * delta;
        }
    }

    for (UINT32 i = 0; i < dim; ++i) {
		for (UINT32 j = 0; j < n; ++j) {
			UINT32 k = RandUint() % n;
			DOUBLE temp = samples[dim*j + i];
			samples[dim*j + i] = samples[dim*k+i];
			samples[dim*k+i] = temp;
		}
	}
}

void SamplerUtil::HaltonSamples(DOUBLE* samples, UINT32 n, UINT32 start) {
    for (UINT32 i = 0; i < n; ++i) {
		*(samples++) = RadicalInverse(i + start, 2);
		*(samples++) = RadicalInverse(i + start, 3);
    }
}

void SamplerUtil::FoldedHaltonSamples(DOUBLE* samples, UINT32 n, UINT32 start) {
    for (UINT32 i = 0; i < n; ++i) {
		*(samples++) = FoldedRadicalInverse(i + start, 2);
		*(samples++) = FoldedRadicalInverse(i + start, 3);
    }
}

void SamplerUtil::Samples02(DOUBLE* samples, UINT32 n) {
    for (UINT32 i = 0; i < n; ++i) {
		*(samples++) = VanDerCorput(i + 1, 3);
		*(samples++) = Sobol(i + 1, 2);
    }
}

}
