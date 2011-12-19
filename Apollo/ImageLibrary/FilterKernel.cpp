#include "Image.h"

namespace Apollo {

void FilterKernel::Normalize() {
	FLOAT sum = 0.0f;
	for (UINT32 i = 0; i < width * height; i++) {
		sum += kernel[i];
	}
	for (UINT32 i = 0; i < width * height; i++) {
		kernel[i] /= sum;
	}		
	normalized = true;
}

void FilterKernel::CreateTriangleFilter2D(FilterKernel* kernel, UINT32 size) {
	kernel->width = kernel->height = size;
	kernel->kernel.resize(size * size);
	UINT32 mid = size / 2;
	for (UINT32 row = 0; row < size; row++) {
		UINT32 rowValue = mid - abs( (INT32)(row - mid));
		for (UINT32 col = 0; col < size; col++) {
			UINT32 colValue = mid - abs((INT32)(col - mid));
			kernel->kernel[row * size + col] = (FLOAT)(1 + rowValue + colValue);
		}
	}
		
	kernel->Normalize();
}
	
void FilterKernel::CreateTriangleFilter1D(FilterKernel* kernel, UINT32 size) {
	kernel->width = size;
	kernel->height = 1;
	kernel->kernel.resize(size);
	UINT32 mid = size / 2;
	for (UINT32 col = 0; col < size; col++) {
		UINT32 colValue = mid - abs((INT32)(col - mid));
		kernel->kernel[col] = (FLOAT)(1 + colValue);
	}
	kernel->Normalize();
}

void FilterKernel::CreateGaussianFilter2D(FilterKernel* kernel, UINT32 size, FLOAT sigma) {
    kernel->width = kernel->height = size;
    kernel->kernel.resize(size * size);
	FLOAT centerX = size / 2.0f;
	FLOAT centerY = size / 2.0f;
	for (UINT32 i = 0; i < size; i++) {
		for (UINT32 j = 0; j < size; j++) {
			FLOAT x = (i - centerX) / centerX;
			FLOAT y = (j - centerY) / centerY;
			FLOAT d = x*x + y*y;
			d = -d / (2*sigma*sigma);
			FLOAT weight = powf((FLOAT)APOLLO_E, d);
			kernel->kernel[i*size + j] = weight;
		}
	}

    kernel->Normalize();
}

}
