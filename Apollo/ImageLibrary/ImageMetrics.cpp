#include "Image.h"

namespace Apollo {

void ImageMetrics::ComputeMeanAndStdDeviation(const Image* image, Color4f& mean, Color4f& stddev) {
    mean = Color4f::ZERO();
    stddev = Color4f::ZERO();;

    UINT32 w = image->GetWidth();
    UINT32 h = image->GetHeight();
    UINT32 pitch = image->GetPitch();
    const Color4f* data = image->GetData();

    for (UINT32 row = 0; row < h; row++) {
        const Color4f* rowData = data + row*pitch;
        for (UINT32 col = 0; col < w; col++) {
            mean += rowData[col];
        }
    }

    mean /= (FLOAT) image->GetSize();

    for (UINT32 row = 0; row < h; row++) {
        const Color4f* rowData = data + row*pitch;
        for (UINT32 col = 0; col < w; col++) {
            Color4f diff = rowData[col] - mean;
            stddev.MultAggregate(diff, diff);
        }
    }

	stddev /= (FLOAT) image->GetSize();

    stddev.r = sqrtf(stddev.r);
    stddev.g = sqrtf(stddev.g);
    stddev.b = sqrtf(stddev.b);
    stddev.a = sqrtf(stddev.a);
}

}
