#pragma once
/**
 * This is a class that will generate nxn grid aligned samples per pixel.
 */

#include "ApolloCommon.h"
#include "PrimarySampler.h"

namespace Apollo {
    class RenderBucket;
    class GridSuperSampler : public PrimarySampler {
	public:
	    GridSuperSampler(UINT32 n);

	    /* Clean up sampler data */
	    virtual ~GridSuperSampler();

	    virtual bool GetNextSample(PrimarySample& sample);
	    virtual void Reset(UINT32 startRow, UINT32 startCol, UINT32 endRow, UINT32 endCol);
	
	private:
	    UINT32	    m_subRow;
	    UINT32	    m_subCol;
	    UINT32	    m_n;
	    UINT32	    m_row;
	    UINT32	    m_col;
	    FLOAT       m_weight;
	    DOUBLE*     m_samples;
    };
}
