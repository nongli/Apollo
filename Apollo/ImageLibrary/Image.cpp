#include "Image.h"

namespace Apollo {

Image::Image(UINT32 width, UINT32 height) :
	m_width(width),
	m_height(height),
	m_pitch(width),
	m_data(nullptr) {
    m_data = (Color4f*)_aligned_malloc(sizeof(Color4f) * m_height * m_pitch, 16);
    if (!m_data) throw ApolloOOMException();
    memset(m_data, 0, sizeof(Color4f)*m_height*m_pitch);
}

Image::Image(const Image& image) :
	m_width(image.GetWidth()),
	m_height(image.GetHeight()),
	m_pitch(image.GetPitch()),
	m_data(nullptr) {
	m_data = (Color4f*)_aligned_malloc(sizeof(Color4f) * m_height * m_pitch, 16);
    if (!m_data) throw ApolloOOMException();
	memcpy(this->GetData(), image.GetData(), m_width * m_height * sizeof(Color4f));
}

Image::Image(ImageData* pData) :
	m_width(pData->uiWidth),
	m_height(pData->uiHeight),
	m_pitch(pData->uiWidth),
	m_data(nullptr) {
	m_data = (Color4f*)_aligned_malloc(sizeof(Color4f) * m_height * m_pitch, 16);
    if (!m_data) throw ApolloOOMException();

	if (pData->format == IMAGE_DATA_FORMAT_BYTE) {
		for (UINT32 i = 0; i < m_width * m_height; i++) {
			m_data[i].r = pData->pByteData[i*4 + 0] / 255.0f;
			m_data[i].g = pData->pByteData[i*4 + 1] / 255.0f;
			m_data[i].b = pData->pByteData[i*4 + 2] / 255.0f;
			m_data[i].a = pData->pByteData[i*4 + 3] / 255.0f;
		}
	} else if (pData->format == IMAGE_DATA_FORMAT_BYTE_BGRA) {
        for (UINT32 i = 0; i < m_width * m_height; i++) {
			m_data[i].b = pData->pByteData[i*4 + 0] / 255.0f;
			m_data[i].g = pData->pByteData[i*4 + 1] / 255.0f;
			m_data[i].r = pData->pByteData[i*4 + 2] / 255.0f;
			m_data[i].a = pData->pByteData[i*4 + 3] / 255.0f;
		}
    } else {
		ApolloException::NotYetImplemented();
	}
}

Image::~Image() {
    if (m_data) _aligned_free(m_data);
}

void Image::CopyData(BYTE* pData, bool includeAlpha, bool bgra) const
{
	if (!pData) ApolloException::NotYetImplemented();
    
    if (!bgra) {
	    for (UINT32 row = 0, index = 0; row < m_height; row++) {
		    for (UINT32 col = 0; col < m_width; col++) {
			    pData[ index++ ] = (BYTE)(m_data[row * m_pitch + col].r * 255);
			    pData[ index++ ] = (BYTE)(m_data[row * m_pitch + col].g * 255);
			    pData[ index++ ] = (BYTE)(m_data[row * m_pitch + col].b * 255);
                if (includeAlpha) {
			        pData[ index++ ] = (BYTE)(m_data[row * m_pitch + col].a * 255);
                }
		    }
	    }
    } else {
        for (UINT32 row = 0, index = 0; row < m_height; row++) {
		    for (UINT32 col = 0; col < m_width; col++) {
			    pData[ index++ ] = (BYTE)(m_data[row * m_pitch + col].b * 255);
			    pData[ index++ ] = (BYTE)(m_data[row * m_pitch + col].g * 255);
			    pData[ index++ ] = (BYTE)(m_data[row * m_pitch + col].r * 255);
                if (includeAlpha) {
			        pData[ index++ ] = (BYTE)(m_data[row * m_pitch + col].a * 255);
                }
		    }
	    }
    }
}

void Image::Blt(const Image* src, const BltArgs* args)
{
	if (!src || !args)
	{
		ApolloException::NotYetImplemented();
	}

    UINT nRows = args->nRows ? args->nRows : src->GetHeight();
    UINT nCols = args->nCols ? args->nCols : src->GetWidth();

	for (UINT nRow = 0; nRow < nRows; nRow++)
	{
		UINT srcIndex = (nRow + args->srcRow) * src->GetPitch() + args->srcCol;
		UINT dstIndex = (nRow + args->dstRow) * this->GetPitch() + args->dstCol;
		memcpy(this->GetData() + dstIndex, src->GetData() + srcIndex, sizeof(Color4f) * nCols);
	}
}

void Image::AlphaBlend(const Image* image, UINT row, UINT col) {
    //TODO clipping???
    UINT w = image->GetWidth();
    UINT h = image->GetHeight();

    Color4f* dstData = this->GetData();
    const Color4f* srcData = image->GetData();
    
    for (UINT r = 0, srcIndex = 0; r < h; r++) {
        UINT dstIndex = (r + row) * m_width + col;
        for (UINT c = 0; c < w; c++, dstIndex++) {
            const Color4f& srcPixel = srcData[srcIndex++];
            FLOAT alpha = srcPixel.a;
            dstData[dstIndex].r = dstData[dstIndex].r * (1 - alpha) + srcPixel.r;
            dstData[dstIndex].g = dstData[dstIndex].g * (1 - alpha) + srcPixel.g;
            dstData[dstIndex].b = dstData[dstIndex].b * (1 - alpha) + srcPixel.b;
        }
    }
}

void Image::RemapLuminance(FLOAT min, FLOAT max) {
    FLOAT imageMin = INFINITY;
    FLOAT imageMax = 0;
    for (UINT i = 0; i < GetSize(); i++) {
        FLOAT l = m_data[i].ToLuminance();
        imageMin = std::min(l, imageMin);
        imageMax = std::max(l, imageMax);
    }

    FLOAT ratio = (max - min) / (imageMax - imageMin);
    for (UINT i = 0; i < GetSize(); i++) {
        FLOAT l = m_data[i].ToLuminance();
        l = (l - min) * ratio + min;
        m_data[i].SetLuminance(l);
    }
}

}
