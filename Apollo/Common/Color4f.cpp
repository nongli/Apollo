#include "Color4f.h"
#include "ApolloMath.h"

namespace Apollo {

static void Matrix3x3Multiply(FLOAT matrix[9], FLOAT a, FLOAT b, FLOAT c, FLOAT& x, FLOAT& y, FLOAT& z) {
	x = matrix[0]*a + matrix[1]*b + matrix[2]*c;	
	y = matrix[3]*a + matrix[4]*b + matrix[5]*c;	
	z = matrix[6]*a + matrix[7]*b + matrix[8]*c;	
}

void Color4f::RGBtoHSV(FLOAT r, FLOAT g, FLOAT b, FLOAT& h, FLOAT& s, FLOAT& v) {
	FLOAT min, max, delta;

	min = MIN3(r, g, b);
	max = MAX3(r, g, b);
	v = max;

	delta = max - min;
	if (max != 0) {
		s = delta / max;
	} else {
		s = 0;
		h = -1;
		return;
	}

	if (r == max) {
		h = (g - b) / delta;
	} else if (g == max) {
		h = 2 + (b-r) / delta;
	} else {
		h = 4 + (r-g) / delta;
	}

	h *= 60;
	if (h < 0) h += 360;
}

void Color4f::HSVtoRGB(FLOAT h, FLOAT s, FLOAT v, FLOAT& r, FLOAT& g, FLOAT& b) {
	int i;
	FLOAT f, p, q, t;
	if (s == 0) {
		r = g = b = v;
		return;
	}

	h /= 60;
	i = (int)floor(h);
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
			r = p; g = p; b = v;
            break;
		case 4:
			r = t; g = p; b = v;
            break;
		default:
			r = v;	g = p; b = q;
			break;
	}
}

void Color4f::RGBtoYIQ(FLOAT r, FLOAT g, FLOAT b, FLOAT& y, FLOAT& i, FLOAT& q) {
	static FLOAT matrix[] = {0.299f,  0.587f,  0.114f,
							 0.596f, -0.275f, -0.321f,
							 0.212f, -0.523f,  0.311f};
	Matrix3x3Multiply(matrix, r, g, b, y, i, q);
}

void Color4f::YIQtoRGB(FLOAT y, FLOAT i, FLOAT q, FLOAT& r, FLOAT& g, FLOAT& b) {
	static FLOAT matrix[] = {1.0f,  0.956f,  0.621f,
							 1.0f, -0.272f, -0.647f,
							 1.0f, -1.105f,  1.702f};
	Matrix3x3Multiply(matrix, y, i, q, r, g, b);
}

void Color4f::RGBtoXYZ(FLOAT r, FLOAT g, FLOAT b, FLOAT& x, FLOAT& y, FLOAT& z) {
	static FLOAT matrix[] = { 0.412453f, 0.357580f, 0.180423f,
							  0.212671f, 0.715160f, 0.072169f,
							  0.019334f, 0.119193f, 0.950227f};
	Matrix3x3Multiply(matrix, r, g, b, x, y, z);
}

void Color4f::XYZtoRGB(FLOAT x, FLOAT y, FLOAT z, FLOAT& r, FLOAT& g, FLOAT& b) {
	static FLOAT matrix[] = { 3.240479f, -1.537150f, -0.498535f,
							 -0.969256f,  1.875992f,  0.041556f,
							  0.055648f, -0.204043f,  1.057311f};
	Matrix3x3Multiply(matrix, x, y, z, r, g, b);
}

}
