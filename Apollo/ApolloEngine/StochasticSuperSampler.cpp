#include "StochasticSuperSampler.h"
#include "RenderBucket.h"
#include "SamplerUtil.h"

namespace Apollo {

bool StochasticSuperSampler::GetNextSample(PrimarySample& sample) {        
    if (m_subIndex == m_n) {
	    SetCurrentPixel(sample);
	    SamplerUtil::RandomSamples(m_samples, m_n * 2);
	    if (!m_bucket->GetNextPixel(m_row, m_col)) return false;
	    m_subIndex  = 0;
    }
        
    SetSampleRowCol(sample, m_row, m_col);
    SetSampleWeight(sample, m_weight);
    sample.SetColor(Color4f::ZERO());
       
    sample.xFilm    = (FLOAT)(m_col + (m_samples[m_subIndex*2 + 0] - 0.5f) * m_pixelSize);
    sample.yFilm    = (FLOAT)(m_row + (m_samples[m_subIndex*2 + 1] - 0.5f) * m_pixelSize);

    m_subIndex++;
    return true;
}

void StochasticSuperSampler::Reset(UINT32 sR, UINT32 sC, UINT32 eR, UINT32 eC) {
    m_bucket->Reset(sR, sC, eR, eC);
    m_subIndex		= m_n;
}

StochasticSuperSampler::StochasticSuperSampler(UINT32 n) : PrimarySampler() {
	m_n		        = n;
	m_subIndex	    = n;
	m_weight	    = 1.0f / (n);
	m_samples       = new DOUBLE[m_n* 2];
}

inline StochasticSuperSampler::~StochasticSuperSampler() {
    SAFE_ARRAY_DELETE(m_samples);
}

}
