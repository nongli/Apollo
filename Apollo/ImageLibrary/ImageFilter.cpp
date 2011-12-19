/**
 *
 * Image filtering.
 *
 */

#include "Image.h"
#include "ApolloCommon.h"

namespace Apollo {

Image* Image::Crop(UINT x, UINT y, UINT w, UINT h) const {
    Image* result = new Image(w, h);
    
    Color4f* dstData = result->GetData();
    const Color4f* srcData = this->GetData() + y * GetPitch() + x;

    for (UINT row = 0; row < h; row++) {
        memcpy(dstData, srcData, sizeof(Color4f) * w);
        srcData += this->GetPitch();
        dstData += result->GetPitch();
    }
    return result;
}

Image* Image::GenerateMipMap() const {
	Image* image = new Image(m_width + m_width / 2, m_height);
	Image* srcImage = this->NaiveScaleToHalf();	

	BltArgs args;
	args.srcRow		= 0;
	args.srcCol		= 0;
	args.nCols		= m_width;
	args.nRows		= m_height;
	args.dstRow		= 0;
	args.dstCol		= 0;

	bool moveRow = false;

	image->Blt(this, &args);
	
	args.dstCol += args.nCols;
	args.nCols /= 2;
	args.nRows /= 2;

	while (args.nCols && args.nRows) {	
		args.nCols = CLAMP(args.nCols, 1, args.nCols);
		args.nRows = CLAMP(args.nRows, 1, args.nRows);

		image->Blt(srcImage, &args);
		srcImage->NaiveScaleToHalf(srcImage);
		
		if (moveRow) args.dstCol += args.nCols;
		else args.dstRow += args.nRows;
		moveRow = !moveRow;
		
		args.nCols /= 2;
		args.nRows /= 2;
	} 
	
	SAFE_DELETE(srcImage);
	return image;
}

Image* Image::NaiveScaleToHalf(Image* dst) const {
	if (this == dst) {
		dst->m_width /= 2;
		dst->m_height /= 2;
	}

	for (UINT32 row = 0; row < dst->GetHeight(); row++) {
		for (UINT32 col = 0; col < dst->GetWidth(); col++) {
			UINT32 dstIndex = row * dst->GetPitch() + col;
			UINT32 srcIndex = (row*2) * this->GetPitch() + (col*2);
			dst->m_data[dstIndex] = m_data[srcIndex];
		}
	}
	return dst;
}

Image* Image::NaiveScaleToHalf() const {
	return this->NaiveScaleToHalf(new Image(m_width / 2, m_height / 2));
}

Image* Image::ToGrayscale() const {	
	return ToGrayscale(new Image(m_width, m_height));
}

Image* Image::ToGrayscale(Image* dst) const {
	if (this->GetWidth() != dst->GetWidth() || this->GetHeight() != dst->GetHeight()) {
		ApolloException::NotYetImplemented();
	}

	for (UINT32 row = 0; row < m_height; row++) {
		for (UINT32 col = 0; col < m_width; col++) {
			const Color4f& pixel = m_data[row * m_pitch + col];
			dst->m_data[row * dst->GetPitch() + col].Set(pixel.ToLuminance());
		}
	}
	return dst;
}

Image* Image::Convolve(FilterKernel* kernel, IMAGE_FILTER_EDGE edge) const {
	return this->Convolve(kernel, edge, new Image(m_width, m_height));
}

Image* Image::Convolve(FilterKernel* kernel , IMAGE_FILTER_EDGE edge, Image* dst) const {
	if (edge == IMAGE_FILTER_EDGE_MIRROR) ApolloException::NotYetImplemented();
	if (kernel->width % 2 == 0 || kernel->height % 2 == 0) ApolloException::NotYetImplemented();

	FLOAT weightSum = 0.0f;

	UINT32 srcPitch = m_pitch;
	UINT32 dstPitch = dst->GetPitch();

	INT32 kWidthHalf = (INT32)(kernel->width / 2);
	INT32 kHeightHalf = (INT32)(kernel->height / 2);

	for (UINT32 row = 0; row < m_height; row++) {
		for (UINT32 col = 0; col < m_width; col++) {
			Color4f pixel;
			UINT32 rowStart = 0;
			UINT32 rowEnd = kernel->height;
			UINT32 colStart = 0;
			UINT32 colEnd = kernel->width;

			if (edge == IMAGE_FILTER_EDGE_IGNORE) {
				rowStart = MAX(0, (INT32)(kHeightHalf - (INT32)row));
				colStart = MAX(0, (INT32)(kWidthHalf - (INT32)col));
				rowEnd = MIN(kernel->height, m_height + kHeightHalf - row);
				colEnd = MIN(kernel->width, m_width + kWidthHalf - col);
				weightSum = 0.0f;
			}
			
			for (UINT32 kRow = rowStart; kRow < rowEnd; kRow++)	{
				for (UINT32 kCol = colStart; kCol < colEnd; kCol++)	{
					INT32 iRow = row + kRow - kHeightHalf;
					INT32 iCol = col + kCol - kWidthHalf;

					if (iRow < 0 || (UINT32)iRow >= m_height) {
						if (edge == IMAGE_FILTER_EDGE_REPEAT) {
							iRow = CLAMP(iRow, 0, (INT32)(m_height - 1));
						}
					}
					if (iCol < 0 || (UINT32)iCol >= m_width) {
						if (edge == IMAGE_FILTER_EDGE_REPEAT) {
							iCol = CLAMP(iCol, 0, (INT32)(m_width - 1));
						}
					}
	
					FLOAT kernelValue = kernel->kernel[kRow * kernel->width + kCol];
					pixel.MultAggregate(m_data[iRow * srcPitch + iCol], kernelValue);
					if (edge == IMAGE_FILTER_EDGE_IGNORE) {
						weightSum += kernelValue;
					}
				}
			}

			if (edge == IMAGE_FILTER_EDGE_IGNORE && kernel->normalized) {
				pixel /= weightSum;
			}

			dst->m_data[row * dstPitch + col] = pixel;
		}
	}
	
	return dst;
}

Image* Image::ConvolveSeperated(FilterKernel* kernel, IMAGE_FILTER_EDGE edge) const {
	return this->ConvolveSeperated(kernel, edge, new Image(m_width, m_height));
}

Image* Image::ConvolveSeperated(FilterKernel* kernel, IMAGE_FILTER_EDGE edge, Image* dst) const {
	if (edge == IMAGE_FILTER_EDGE_MIRROR) ApolloException::NotYetImplemented();
	if (kernel->width % 2 == 0) ApolloException::NotYetImplemented();

	FLOAT weightSum = 0;
	UINT32 srcPitch = m_pitch;
	UINT32 dstPitch = m_width;
	
	UINT32 kSize = kernel->width;
	INT32 kHalf = (INT32)(kSize / 2);

    Color4f* tempData = Color4f::Allocate(m_width * m_height);

	for (UINT32 row = 0; row < m_height; row++) {
		for (UINT32 col = 0; col < m_width; col++) {
			Color4f pixel;

			UINT32 colStart = 0;
			UINT32 colEnd = kSize;

			if (edge == IMAGE_FILTER_EDGE_IGNORE) {
				colStart = MAX(0, (INT32)(kHalf - (INT32)col));
				colEnd = MIN(kSize, m_width + kHalf - col);
				weightSum = 0.0f;
			}

			for (UINT32 kCol = colStart; kCol < colEnd; kCol++) {
				INT32 iCol = col + kCol - kHalf;
					
				if (iCol < 0 || (UINT32)iCol >= m_width) {
					if (edge == IMAGE_FILTER_EDGE_REPEAT) {
						iCol = CLAMP(iCol, 0, (INT32)(m_width - 1));
					}
				}
	
				FLOAT kernelValue = kernel->kernel[kCol];
				pixel.MultAggregate(m_data[row*srcPitch + iCol], kernelValue);

				if (edge == IMAGE_FILTER_EDGE_IGNORE) {
					weightSum += kernelValue;
				}
			}

			if (edge == IMAGE_FILTER_EDGE_IGNORE && kernel->normalized) {
				pixel /= weightSum;	
			}
			
			tempData[row * dstPitch + col] = pixel;			
		}
	}

	srcPitch = m_width;
	dstPitch = dst->GetPitch();

	for (UINT32 row = 0; row < m_height; row++) {
		for (UINT32 col = 0; col < m_width; col++) {
			Color4f pixel;
			UINT32 rowStart = 0;
			UINT32 rowEnd = kSize;

			if (edge == IMAGE_FILTER_EDGE_IGNORE) {
				rowStart	= MAX(0, (INT32)(kHalf - (INT32)row));
				rowEnd		= MIN(kSize, m_height + kHalf - row);
				weightSum	= 0.0f;
			}

			for (UINT32 kRow = rowStart; kRow < rowEnd; kRow++) {
				INT32 iRow = row + kRow - kHalf;
					
				if (iRow < 0 || (UINT32)iRow >= m_height) {
					if (edge == IMAGE_FILTER_EDGE_REPEAT) {
						iRow = CLAMP(iRow, 0, (INT32)(m_height - 1));
					}
				}
	
				FLOAT kernelValue = kernel->kernel[kRow];
				pixel.MultAggregate(tempData[iRow * srcPitch + col], kernelValue);

				if (edge == IMAGE_FILTER_EDGE_IGNORE) {
					weightSum += kernelValue;
				}
			}

			if (edge == IMAGE_FILTER_EDGE_IGNORE && kernel->normalized) {
				pixel /= weightSum;
			}
			dst->m_data[row * dstPitch + col] = pixel;
		}
	}
	
    Color4f::Free(tempData);
	return dst;
}

}
