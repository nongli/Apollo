#pragma once

namespace Apollo
{
	void RandSeed(UINT32);
	UINT32 RandUint();
	DOUBLE RandDouble();	
	INT32 RandInt(INT32 min, INT32 max);
	UINT32 RandInt(UINT32 max);

	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	// Implementation below
	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	#define MATRIX_A 0x9908b0dfUL
	#define LOWER_MASK 0x80000000UL
	#define UPPER_MASK 0x7fffffffUL
	#define RAND_N 624
	#define RAND_M 397

	static UINT32 mti = RAND_N + 1;
	static UINT32 mt[RAND_N];

	static UINT32 mag01[2] = {0x0UL, MATRIX_A};

	inline void RandSeed(UINT32 seed)
	{
		mt[0] = seed & 0xffffffffUL;
		for (mti = 1; mti < RAND_N; mti++) {
			mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
			mt[mti] &= 0xffffffffUL;
		}
	}

	inline UINT32 RandUint()
	{
		UINT32 y;

		if (mti >= RAND_N) 
		{
			UINT32 kk;

			if (mti == RAND_N + 1)
			{
				RandSeed(5489UL);
			}

			for (kk = 0; kk < RAND_N - RAND_M; kk++) 
			{
				y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
				mt[kk] = mt[kk + RAND_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
			}

			for (; kk < RAND_N - 1; kk++) 
			{
				y = (mt[kk] & UPPER_MASK | mt[kk + 1] & LOWER_MASK);
				mt[kk] = mt[kk + (RAND_M - RAND_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
			}

			y = (mt[RAND_N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
			mt[RAND_N -1] = mt[RAND_M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

			mti = 0;
		}

		y = mt[mti++];

		y ^= (y >> 11);
		y ^= (y << 7) & 0x9d2c5680UL;
		y ^= (y << 15) & 0xefc60000UL;
		y ^= (y >> 18);

		return y;
	}

	inline DOUBLE RandDouble()
	{
		return RandUint() * UINT_MAX_INV;
	}

	inline INT32 RandInt(INT32 min, INT32 max)
	{
		return (INT32) (RandDouble() * (max - min)) + min;
	}

	inline UINT32 RandInt(UINT32 max)
	{
		return (UINT32) (RandDouble() * max);
	}
};