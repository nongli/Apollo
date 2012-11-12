#pragma once

#include <xmmintrin.h>
#include "ApolloMath.h"

namespace Apollo {

__declspec(align(16)) class Color4f {
public:
	
#pragma warning( disable : 4201 )
	__declspec(align(16)) union {
		__declspec(align(16)) struct {
			FLOAT r, g, b, a;
		};        
		__m128 simdData;
	};	
#pragma warning( default : 4201 )
	
    static Color4f* Allocate(size_t numElements) {
        return (Color4f*)_aligned_malloc(numElements * sizeof(Color4f), sizeof(Color4f));
    }

    static void Free(Color4f* mem) {
        _aligned_free(mem);
    }

	Color4f() : r(0), g(0), b(0), a(0) {}    
	Color4f(FLOAT _r, FLOAT _g, FLOAT _b) : r(_r), g(_g), b(_b), a(1) {}
	Color4f(FLOAT _r, FLOAT _g, FLOAT _b, FLOAT _a) : r(_r), g(_g), b(_b), a(_a) {}

	inline Color4f& operator= (const Color4f& rhs) {
		r = rhs.r; 
		g = rhs.g;
		b = rhs.b;
		a = rhs.a;
		return *this;
	}

	inline Color4f& Set(FLOAT v) {
		simdData = _mm_set1_ps(v);
		a = 1.0f;
		return *this;
	}

	inline Color4f& Set(FLOAT _r, FLOAT _g, FLOAT _b) {
		r = _r;
		g = _g;
		b = _b;
		a = 1.0f;
		return *this;
	}

	inline Color4f& Set(FLOAT _r, FLOAT _g, FLOAT _b, FLOAT _a) {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
		return *this;
	}

	inline Color4f operator*(FLOAT s) const { return Color4f(r*s, g*s, b*s, a*s); }
	inline Color4f operator*(const Color4f& rhs) const { return Color4f(r*rhs.r, g*rhs.g, b*rhs.b, a*rhs.a); }
	inline Color4f operator+(const Color4f& rhs) const { return Color4f(r+rhs.r, g+rhs.g, b+rhs.b, a+rhs.a); }

	inline Color4f& operator+= (const Color4f& rhs) { 
		simdData = _mm_add_ps(simdData, rhs.simdData);
		return *this;
	}

    inline Color4f& operator *= (FLOAT s) {
        r *= s; g *= s; b *= s; a *= s;
        return *this;
    }

    inline Color4f operator- (const Color4f& rhs) const {
        return Color4f(r - rhs.r, g - rhs.g, b - rhs.b, a - rhs.a);
    }

	inline Color4f& operator/= (const FLOAT s) {
		__m128 sSIMD = _mm_set1_ps(s);
		simdData = _mm_div_ps(simdData, sSIMD);
		return *this;
	}
    
    inline FLOAT MagnitudeRGB() const {
        return r*r + g*g + b*b;
    }

    inline FLOAT Distance2(const Color4f& other) const {
        FLOAT dR = r - other.r;
        FLOAT dG = g - other.g;
        FLOAT dB = b - other.b;
        FLOAT dA = a - other.a;
        return dR*dR + dG*dG + dB*dB + dA*dA;
    }

    inline FLOAT Distance2RGB(const Color4f& other) const {
        FLOAT dR = r - other.r;
        FLOAT dG = g - other.g;
        FLOAT dB = b - other.b;
        return dR*dR + dG*dG + dB*dB;
    }

    inline void ClampRGB() {
        r = CLAMP(r, 0, 1.0f);
        g = CLAMP(g, 0, 1.0f);
        b = CLAMP(b, 0, 1.0f);
    }

    inline void Abs() {
        r = fabs(g);
        g = fabs(g);
        b = fabs(b);
        a = fabs(a);
    }

	inline void MultAggregate(const Color4f& src, FLOAT s) {
		/*
		__m128 sSIMD = _mm_set1_ps(s);
		simdData = _mm_add_ps(simdData, _mm_mul_ps(src.simdData, sSIMD));		
		*/
		r += src.r * s;
		g += src.g * s;
		b += src.b * s;
		a += src.a * s;
	}

	inline void MultAggregate(const Color4f& src, const Color4f& s)	{
		simdData = _mm_add_ps(simdData, _mm_mul_ps(src.simdData, s.simdData));
	}

	inline FLOAT ToLuminance() const {
		return r*0.39f + g*0.50f + b*0.11f;
	}

    inline FLOAT MaxChannel() const {
        return MAX(MAX(r, g), MAX(b, a));
    }

    inline FLOAT MaxChannelRGB() const {
        return MAX(MAX(r, g), b);
    }

    inline void SetLuminance(FLOAT lum) {       				
		FLOAT y, i, q;

		RGBtoYIQ(r, g, b, y, i, q);
		YIQtoRGB(lum, i, q, r, g, b);

        r = CLAMP(r, 0.0f, 1.0f);
        g = CLAMP(g, 0.0f, 1.0f);
        b = CLAMP(b, 0.0f, 1.0f);
    }
        
    inline static const Color4f& ZERO() {
	    static Color4f c(0, 0, 0, 0);
	    return c;
    }
	inline static const Color4f& BLACK() {
	    static Color4f c(0, 0, 0);
	    return c;
    }
	inline static const Color4f& WHITE() {
		static Color4f c(1, 1, 1);
		return c;
	}
    inline static const Color4f& RED() {
        static Color4f c(1, 0, 0);
        return c;
    }
    inline static const Color4f& GREEN() {
        static Color4f c(0, 1, 0);
        return c;
    }
    inline static const Color4f& BLUE() {
        static Color4f c(0, 0, 1);
        return c;
    }

    inline static Color4f FromRGB(int r, int g, int b, int a = 255) {
        return Color4f(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    inline static Color4f FromHSV(FLOAT h, FLOAT s, FLOAT v) {
        UINT32 i;
	    FLOAT f, p, q, t;
        FLOAT r, g, b;

	    if (s == 0) {
		    r = g = b = v;
		    return Color4f(r, g, b);
	    }

	    h /= 60;
	    i = (UINT32)floor(h);
	    f = h - i;
	    p = v * (1 - s);
	    q = v * (1 - s*f);
	    t = v * (1 - s*(1-f));

	    switch (i) {
		    case 0:
			    r = v; g = t; b = p;
			    break;
		    case 1:
			    r = q; g = v; b = p;
			    break;
		    case 2:
			    r = p; g = v; b = t;
			    break;
		    case 3:
			    r = p; g = q; b = v;
                break;
		    case 4:
			    r = t; g = p; b = v;
                break;
		    default:
			    r = v; g = p; b = q;
			    break;
	    }
        return Color4f(r, g, b);
    }

    static void RGBtoHSV(FLOAT r, FLOAT g, FLOAT b, FLOAT& h, FLOAT& s, FLOAT& v);
    static void HSVtoRGB(FLOAT h, FLOAT s, FLOAT v, FLOAT& r, FLOAT& g, FLOAT& b);
    static void RGBtoYIQ(FLOAT r, FLOAT g, FLOAT b, FLOAT& y, FLOAT& i, FLOAT& q);
    static void YIQtoRGB(FLOAT y, FLOAT i, FLOAT q, FLOAT& r, FLOAT& g, FLOAT& b);
    static void RGBtoXYZ(FLOAT r, FLOAT g, FLOAT b, FLOAT& x, FLOAT& y, FLOAT& z);
    static void XYZtoRGB(FLOAT x, FLOAT y, FLOAT z, FLOAT& r, FLOAT& g, FLOAT& b);
};

}
