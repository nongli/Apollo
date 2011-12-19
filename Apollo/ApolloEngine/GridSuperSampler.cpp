#include "GridSuperSampler.h"
#include "RenderBucket.h"
#include "SamplerUtil.h"

namespace Apollo {

bool GridSuperSampler::GetNextSample(PrimarySample& sample) {        
    if (m_subRow == m_n) {
	    SetCurrentPixel(sample);
	    if (!m_bucket->GetNextPixel(m_row, m_col)) return false;
	    m_subRow    = 0;
	    m_subCol    = 0;
    }

    SetSampleRowCol(sample, m_row, m_col);
    SetSampleWeight(sample, m_weight);
    sample.SetColor(Color4f::ZERO());

    UINT32	index	= m_subRow*m_n + m_subCol;

    sample.xFilm    = (FLOAT)(m_col + (m_samples[index*2 + 0] - .5f) * m_pixelSize);
    sample.yFilm    = (FLOAT)(m_row + (m_samples[index*2 + 1] - .5f) * m_pixelSize);


    m_subCol++;
    if (m_subCol == m_n) { 
	    m_subCol = 0;
	    m_subRow++;
    }
    return true;
}

void GridSuperSampler::Reset(UINT32 sR, UINT32 sC, UINT32 eR, UINT32 eC) {
    m_bucket->Reset(sR, sC, eR, eC);
    m_subCol		= m_n;
    m_subRow		= m_n;
}

GridSuperSampler::GridSuperSampler(UINT32 n) : PrimarySampler() {
	m_n		    = n;
	m_subRow	= n;
	m_subCol	= n;
	m_weight	= 1.0f / (n*n);

	m_samples   = new DOUBLE[m_n * m_n * 2];
	SamplerUtil::GridSamples(m_samples, m_n, m_n);
}

GridSuperSampler::~GridSuperSampler() {
    SAFE_ARRAY_DELETE(m_samples);
}

}
