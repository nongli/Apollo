#include "PrimarySampler.h"
#include "RenderBucket.h"

namespace Apollo {

bool PrimarySampler::GetNextSample(PrimarySample& sample) {
    sample.SetCurrentPixel();
        
    UINT32  row;
    UINT32  col;
    if (!m_bucket->GetNextPixel(row, col)) return false;
        
    sample.xFilm    = (FLOAT)col;
    sample.yFilm    = (FLOAT)row;
	SetSampleRowCol(sample, row, col);
	SetSampleWeight(sample, 1);
    sample.SetColor(Color4f::ZERO());
    return true;
}

void PrimarySampler::Reset(UINT32 sR, UINT32 sC, UINT32 eR, UINT32 eC) {
    m_bucket->Reset(sR, sC, eR, eC);
}

PrimarySample::PrimarySample(Image* image) {
	row	        = -1;
	col	        = -1;
	m_image	    = image;
    model       = nullptr;
	pixelColor  = Color4f::ZERO();
}

void PrimarySample::SetCurrentPixel() {
    if (row >= 0 && col >= 0) {
        pixelColor.ClampRGB();
        m_image->SetPixel(pixelColor, row, col);
	    pixelColor = Color4f::ZERO();
    }
}

void PrimarySample::SetColor(const Color4f& c){
    sampleColor = c;
    pixelColor.MultAggregate(c, weight);
}

PrimarySampler::PrimarySampler() {
	m_bucket    = nullptr;
    m_pixelSize = 1.0f;
}

PrimarySampler::~PrimarySampler() {
	// Noop
}

void PrimarySampler::SetRenderBucket(RenderBucket* b) {
	m_bucket    = b;
}

void PrimarySampler::SetPixelSize(FLOAT pixelSize) {
    m_pixelSize = pixelSize;
}

FLOAT PrimarySampler::GetPixelSize() const {
    return m_pixelSize;
}

}
