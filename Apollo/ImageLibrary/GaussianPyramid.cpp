#include "Image.h"

namespace Apollo {

GaussianPyramid* GaussianPyramid::Create(const Image* image) {
    return new GaussianPyramid(image);
}

GaussianPyramid::GaussianPyramid(const Image* image) {
    m_images.push_back(new Image(*image));

    const Image* oldImage = image;
    UINT32 width = oldImage->GetWidth() / 2;
    UINT32 height = oldImage->GetHeight() / 2;

    FilterKernel kernel;
    FilterKernel::CreateGaussianFilter2D(&kernel, 9, .25);

    while (width != 0 && height != 0) {
        width = CLAMP(width, 0, 1);
        height = CLAMP(height, 0, 1);

        Image* filteredImage = oldImage->Convolve(&kernel, Image::IMAGE_FILTER_EDGE_IGNORE);
        Image* newImage = filteredImage->NaiveScaleToHalf();
        m_images.push_back(newImage);
        delete filteredImage;

        oldImage = newImage;
        width = width / 2;
        height = height / 2;
    }
}

GaussianPyramid::~GaussianPyramid() {
    for(UINT i = 0; i < m_images.size(); i++) {
        delete m_images[i];
    }
    m_images.clear();
}
    
UINT32 GaussianPyramid::GetNumLevels() const {
    return m_images.size();
}
    
const Image* GaussianPyramid::GetImageAt(UINT32 level) const {
    return m_images[level];
}

Image* GaussianPyramid::ToImage() const {
    if (m_images.size() == 0) return nullptr;
    if (m_images.size() == 1) return new Image(*m_images[0]);
                
    Image* srcImage = m_images[0];
    int w = srcImage->GetWidth();
    int h = srcImage->GetHeight();
    bool moveCol = true;

    Image* result = new Image(w + w/2, h);

    BltArgs args;
	args.srcRow		= 0;
	args.srcCol		= 0;
	args.nCols		= w;
	args.nRows		= h;
	args.dstRow		= 0;
	args.dstCol		= 0;
        
	for (size_t i = 0; i < m_images.size(); i++) {
        result->Blt(m_images[i], &args);	
        if (moveCol) {
			args.dstCol += args.nCols;
		} else {
			args.dstRow += args.nRows;
		}
		moveCol = !moveCol;
        args.nCols /= 2;
	    args.nRows /= 2;
    }

    return result;
}

}
