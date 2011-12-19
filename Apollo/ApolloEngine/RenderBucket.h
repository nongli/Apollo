#pragma once
/**
 * This is the subclass for different strategies to control the pixel order.  Typical
 * strategies are scanline order or tiling.
 */
#include "ApolloCommon.h"

namespace Apollo {
    class RenderBucket {
	public:
	    virtual ~RenderBucket();
	    
	    virtual bool GetNextPixel(UINT32& row, UINT32& col) = 0;
	    virtual void Reset(UINT32 startRow, UINT32 startCol, UINT32 endRow, UINT32 endCol) = 0;
	    virtual FLOAT GetPercentageComplete() const = 0;
    };

    inline RenderBucket::~RenderBucket() {}
}
