#include "Image.h"
#include "ApolloCommon.h"

namespace Apollo {

CannyEdgeDetect::CannyEdgeDetect(const Image* image, EdgeDetectOperator::Operator op) : 
    m_width(image->GetWidth()), m_height(image->GetHeight()) {
	FilterKernel kernel(5, 5);
	kernel.kernel[0] = 2;
	kernel.kernel[1] = 4;
	kernel.kernel[2] = 5;
	kernel.kernel[3] = 4;
	kernel.kernel[4] = 2;
		
	kernel.kernel[5] = 4;
	kernel.kernel[6] = 9;
	kernel.kernel[7] = 12;
	kernel.kernel[8] = 9;
	kernel.kernel[9] = 4;
		
	kernel.kernel[10] = 5;
	kernel.kernel[11] = 12;
	kernel.kernel[12] = 15;
	kernel.kernel[13] = 12;
	kernel.kernel[14] = 5;
		
	kernel.kernel[15] = 4;
	kernel.kernel[16] = 9;
	kernel.kernel[17] = 12;
	kernel.kernel[18] = 9;
	kernel.kernel[19] = 4;
		
	kernel.kernel[20] = 2;
	kernel.kernel[21] = 4;
	kernel.kernel[22] = 5;
	kernel.kernel[23] = 4;
	kernel.kernel[24] = 2;

	kernel.Normalize();

	Image* grayscale = image->ToGrayscale();
	Image* filtered = grayscale->Convolve(&kernel, Image::IMAGE_FILTER_EDGE_IGNORE);
	delete grayscale;

	m_edges.resize(filtered->GetWidth() * filtered->GetHeight());
	if (op == EdgeDetectOperator::Default) {
		op = EdgeDetectOperator::Sobel;
	}
	ComputeEdges(filtered, op);
	Apollo::ImageIO::Save("E:\\Data\\filtered.png", filtered);
	delete filtered;
}

void CannyEdgeDetect::ComputeEdges(const Image* image, EdgeDetectOperator::Operator op) {
	if (op == EdgeDetectOperator::Sobel) {
		FilterKernel yKernel(3, 3);
		FilterKernel xKernel(3, 3);
			
		yKernel.kernel[0] = -1;
		yKernel.kernel[1] = -2;
		yKernel.kernel[2] = -1;
		yKernel.kernel[3] = 0;
		yKernel.kernel[4] = 0;
		yKernel.kernel[5] = 0;
		yKernel.kernel[6] = 1;
		yKernel.kernel[7] = 2;
		yKernel.kernel[8] = 1;

		xKernel.kernel[0] = -1;
		xKernel.kernel[1] = 0;
		xKernel.kernel[2] = 1;
		xKernel.kernel[3] = -2;
		xKernel.kernel[4] = 0;
		xKernel.kernel[5] = 2;
		xKernel.kernel[6] = -1;
		xKernel.kernel[7] = 0;
		xKernel.kernel[8] = 1;

		Image* gY = image->Convolve(&yKernel, Image::IMAGE_FILTER_EDGE_IGNORE);
		Image* gX = image->Convolve(&xKernel, Image::IMAGE_FILTER_EDGE_IGNORE);

		for (UINT32 index = 0; index < image->GetSize(); index++) {
			FLOAT gx = gX->GetPixel(index).r;
			FLOAT gy = gY->GetPixel(index).r;
			FLOAT g = sqrt(gx*gx + gy*gy);
            EdgeDirection::Direction dir = ComputeEdgeDirection(gx, gy);
            m_edges[index] = Edge(gx, gy, g, dir);
		}
			
		delete gY;
		delete gX;
	} else {
		ApolloException::NotYetImplemented();
	}
}

CannyEdgeDetect::EdgeDirection::Direction CannyEdgeDetect::ComputeEdgeDirection(FLOAT gx, FLOAT gy) const {
	FLOAT theta = atan2f(gx, gy);
	FLOAT degree = (FLOAT) RAD_TO_DEGREE(theta);
	assert(degree >= -180 && degree <= 180);

	if (degree >= -22.5f && degree <= 22.5f) {
		return EdgeDirection::Horizontal;
	} else if (degree >= 22.5f && degree < 67.5f) {
		return EdgeDirection::Angle45;
	} else if (degree >= 67.5f && degree < 112.5f) {
		return EdgeDirection::Vertical;
	} else if (degree >= 112.5 && degree < 157.5f) {
		return EdgeDirection::Angle135;
	} else if (degree >= 157.5f || degree < -157.5f) {
		return EdgeDirection::Horizontal;
	} else if (degree >= -157.5f && degree < -112.5f) {
		return EdgeDirection::Angle45;
	} else if (degree >= -112.5f && degree < -67.5f) {
		return EdgeDirection::Vertical;
	} else if (degree >= -67.5 && degree < -22.5f) {
		return EdgeDirection::Angle135;
	}
    throw ApolloException("Whoops");
}

Image* CannyEdgeDetect::GetEdgeImage() const {
	Image* image = new Image(m_width, m_height);
	Color4f* pixels = image->GetData();

	for (UINT index = 0; index < image->GetSize(); index++) {
		pixels[index].r = pixels[index].g = pixels[index].b = m_edges[index].g;
		pixels[index].a = 1;
	}

    Color4f mean, stddev;
    ImageMetrics::ComputeMeanAndStdDeviation(image, mean, stddev);
	for (UINT row = 0; row < m_height; row++) {
		for (UINT col = 0; col < m_width; col++) {
			UINT index = row * m_width + col;
            FLOAT g = m_edges[index].g;

			// Non-maximal suppression
            switch (m_edges[index].dir) {
            case EdgeDirection::Horizontal:
				if ((col > 0 && m_edges[index - 1].g > g) || (col < m_width - 1 && m_edges[index + 1].g > g)) {
                    pixels[index] = Color4f::ZERO();
				}
				break;
			case EdgeDirection::Vertical:
				if ((row > 0 && m_edges[index - m_width].g > g) || (row < m_height - 1 && m_edges[index + m_width].g > g)) {
                    pixels[index] = Color4f::ZERO();
				}
				break;
			case EdgeDirection::Angle135:
				if (col < m_width - 1 && row > 0 && m_edges[index + 1 - m_width].g > g) {
                    pixels[index] = Color4f::ZERO();
				}
				if (col > 0 && row < m_height - 1 && m_edges[index - 1 + m_width].g > g) {
                    pixels[index] = Color4f::ZERO();
				}
				break;
			case EdgeDirection::Angle45:
				if (col < m_width - 1 && row < m_height - 1 && m_edges[index + 1 + m_width].g > g) {
                    pixels[index] = Color4f::ZERO();
				}
				if (col > 0 && row > 0 && m_edges[index - 1 - m_width].g > g) {
                    pixels[index] = Color4f::ZERO();
				}
				break;
			}
		}
	}

    /*
    for (UINT index = 0; index < m_width * m_height; index++) {
        if (pixels[index].r < 20.0f / 255.0f) {
            pixels[index] = Color4f::BLACK();
        } else if (pixels[index].r > 80.0f / 255.0f) {
            pixels[index] = Color4f::WHITE();
        } else {
            pixels[index] = Color4f(.5f, .5f, .5f);
        }
    }
    */

    image->RemapLuminance(0, 1);
    return image;
}

}
