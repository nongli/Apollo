#pragma once
/**
 * This is a class that will generate n random samples per pixel.
 */
#include "ApolloCommon.h"
#include "PrimarySampler.h"

namespace Apollo {
    class RenderBucket;

    class StochasticSuperSampler : public PrimarySampler {
	public:
	    StochasticSuperSampler(UINT32 n);
	    virtual ~StochasticSuperSampler();

	    virtual bool GetNextSample(PrimarySample& sample);
	    virtual void Reset(UINT32 startRow, UINT32 startCol, UINT32 endRow, UINT32 endCol);
	
	private:
	    UINT32	    m_subIndex;
	    UINT32	    m_n;
	    UINT32	    m_row;
	    UINT32	    m_col;
	    FLOAT       m_weight;
	    DOUBLE*     m_samples;
    };
}
