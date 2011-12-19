/**
 * This contins various static methods that will sample from different
 * distributions
 */
#pragma once

#include "ApolloCommon.h"

namespace Apollo {
    class SamplerUtil {
	public:
	    static void Init(bool randSeed=true);

	    static Vector3 RandSphereSample();
	    static Vector3 RandSphereSample(DOUBLE u, DOUBLE v);
	    static Vector3 RandHemiSphereSample(const Vector3& normal);
	    static Vector3 RandHemiSphereCosSample(const Vector3& normal);
	    
	    static Vector3 RandHemiSphereCosSample(const Vector3& normal,
		    DOUBLE u, DOUBLE v);

	    static Vector3 RandHemiSphereSample(const Vector3& normal,
		    DOUBLE u, DOUBLE v);

	    static void RandDiskSample(DOUBLE u, DOUBLE v, DOUBLE& x, DOUBLE& y);

	    static void RandConcentricDiskSample(DOUBLE u, DOUBLE v, DOUBLE&x,
		    DOUBLE& y);

	    // base should be the nth prime
	    static DOUBLE FoldedRadicalInverse(UINT32 n, UINT32 base);
	    static DOUBLE RadicalInverse(UINT32 n, UINT32 base);

	    static DOUBLE VanDerCorput(UINT32 n, UINT32 scramble);
	    static DOUBLE Sobol(UINT32 n, UINT32 scramble);

	    // fill in random samples - memory should be allocated
	    static void RandomSamples(DOUBLE* samples, UINT32 n);
	    static void GridSamples(DOUBLE* samples, UINT32 x, UINT32 y);
	    static void JitteredSamples(DOUBLE* samples, UINT32 x, UINT32 y);
	    static void MultiJitteredSamples(DOUBLE* samples, UINT32 x, UINT32 y);
	    static void LHCSamples(DOUBLE* samples, UINT32 n, UINT32 dim);
	    static void HaltonSamples(DOUBLE* samples, UINT32 n, UINT32 start=1);
	    static void FoldedHaltonSamples(DOUBLE* samples, UINT32 n, UINT32 start=1);
	    static void Samples02(DOUBLE* samples, UINT32 n);
    };

	
    inline Vector3 SamplerUtil::RandHemiSphereCosSample(const Vector3& n) {
		DOUBLE	u		= RandDouble();
		DOUBLE	v		= RandDouble();

		DOUBLE theta	= acos(sqrt(u));
		DOUBLE phi		= 2 * APOLLO_PI * v;

		Vector3 U		= n.GetDirection();
		Vector3 V		= Vector3::XVEC();

		if (fabs(U.Dot(V)) > .9) V = Vector3::YVEC();
		
		Vector3 W	= U.Cross(V).GetDirection();
		V			= W.Cross(U);
		return cos(theta)*U + sin(theta) * (cos(phi)*V + sin(phi)*W);
    }

    inline DOUBLE SamplerUtil::RadicalInverse(UINT32 n, UINT32 base) {
		DOUBLE val = 0;
		DOUBLE invBase = 1.0 / base, invBi = invBase;
		while (n > 0) {
			UINT32 d = n % base;
			val += d * invBi;
			n /= base;
			invBi *= invBase;
		}
		return val;
    }

    inline DOUBLE SamplerUtil::FoldedRadicalInverse(UINT32 n, UINT32 base) {
		DOUBLE val = 0;
		DOUBLE invBase = 1.0 / base, invBi = invBase;
		UINT32 modOffset = 0;
		while (val + base * invBi != val) {
			UINT32 d = (n+modOffset) % base;
			val += d * invBi;
			n /= base;
			invBi *= invBase;
			++modOffset;
		}
		return val;
    }

    inline DOUBLE SamplerUtil::VanDerCorput(UINT32 n, UINT32 scramble) {
		n = (n << 16) | (n >> 16);
		n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
		n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
		n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
		n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
		n ^= scramble;
		return DOUBLE(n) / DOUBLE(0x100000000LL);
    }

    inline DOUBLE SamplerUtil::Sobol(UINT32 n, UINT32 scramble) {
		for (INT32 v = 1 << 31; n != 0; n >>= 1, v ^= v >> 1) {
			if (n & 0x1) scramble ^= v;
        }
		return DOUBLE(scramble) / DOUBLE(0x100000000LL); 
    }
}
