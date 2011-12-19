#pragma once
/**
 * Simple bucketing strategy.  Generates the resulting image in scan line order.
 */

#include "ApolloCommon.h"
#include "RenderBucket.h"

namespace Apollo {
    class ScanLineBucket : public RenderBucket {
	public:
	    ScanLineBucket(UINT32 startRow, UINT32 startCol, UINT32 endRow, UINT32 endCol);
	    ScanLineBucket(UINT32 width, UINT32 height);

	    /* Clean up bucket data */
        virtual ~ScanLineBucket() {}
	    
	    virtual bool GetNextPixel(UINT32& row, UINT32& col);
	    virtual void Reset(UINT32 startRow, UINT32 startCol, UINT32 endRow, UINT32 endCol);

	    virtual FLOAT GetPercentageComplete() const;

	private:
        UINT32      m_sRow, m_eRow;
        UINT32      m_sCol, m_eCol;
	    UINT32	    m_row;
	    UINT32	    m_col;

        UINT32      m_done;
        UINT32      m_total;
    };

    inline ScanLineBucket::ScanLineBucket(UINT32 width, UINT32 height) {
	    Reset(0, 0, width, height);
    }

    inline ScanLineBucket::ScanLineBucket(UINT32 sR, UINT32 sC, UINT32 eR, UINT32 eC) {
	    Reset(sR, sC, eR, eC);
    }

    inline void ScanLineBucket::Reset(UINT32 sR, UINT32 sC, UINT32 eR, UINT32 eC) {
    	m_eCol	    = eC;
	    m_eRow      = eR;
	    m_sRow	    = sR;
	    m_sCol	    = sC;

        m_total = (eR - sR) * (eC - sC);
        m_done = 0;

        m_row = m_sRow;
        m_col = m_sCol;
    }

    inline bool ScanLineBucket::GetNextPixel(UINT32& row, UINT32& col) {
	    if (m_row == m_eRow) return false;
	
	    row	= m_row;
	    col	= m_col;
	    m_col++;
	    if (m_col == m_eCol) {
	        m_col = m_sRow;
	        m_row++;
	    }
        m_done++;
	    return true;
    }

    inline FLOAT ScanLineBucket::GetPercentageComplete() const {
        return ((FLOAT)m_done) / m_total;
    }
}
