#pragma once
/**
 * This is the superclass for samplers for primary rays.  This abstraction makes it
 * possible to combine similar effects (motion blur, antialiasing, depth of 
 * field, etc).  
 *
 * The design is a little round about to allow for different types of antialiasing
 * algorithms.  
 *
 * The user should call get next sample repeatedly passing it a PrimarySample.
 * Some of the fields in the PrimarySample will be immutable and the sample
 * can look at it if dynamic sampling strategies are used (i.e. adaptive
 * supersampling).
 *
 * The primary sampler will generate one sample per pixel through the center
 * of the pixel.
 *
 */

#include "EngineCommon.h"

namespace Apollo {
    class Camera;
    class RenderBucket;
    class Model;

    class PrimarySample {
    public:
	    PrimarySample(Image* image);
	    
        void SetColor(const Color4f&);
        const Color4f& GetSampleColor() const { return sampleColor; }
        
        FLOAT               xFilm;
	    FLOAT               yFilm;
        INT32	            row;
	    INT32	            col;
        const Model*        model;

    private:
        void SetCurrentPixel();
	    
        Color4f     sampleColor;
	    Color4f     pixelColor;
	    FLOAT       weight;

	private:
	    friend class PrimarySampler;
	    Image*      m_image;
    };

    
    class PrimarySampler {
   	public:
	    PrimarySampler();

	    virtual ~PrimarySampler();

	    //  Sets the sampler to have new bounds
	    virtual void Reset(UINT32 startRow, UINT32 startCol, UINT32 endRow, UINT32 endCol);
	   
        // Gets the next sample.  Returns false if it is the end of the render
	    virtual bool GetNextSample(PrimarySample& sample);

	    // Sets the render bucket for the sampler
	    virtual void SetRenderBucket(RenderBucket* b);

        // Pixel size defaults to 1.  Making it larger will blur the image
        // but reduce aliases.
        void SetPixelSize(FLOAT pixelSize);
        FLOAT GetPixelSize() const;

	protected:
        void SetCurrentPixel(PrimarySample& sample) { sample.SetCurrentPixel(); }
        void SetSampleRowCol(PrimarySample& sample, UINT32 r, UINT32 c) { sample.row = r; sample.col = c; }
        void SetSampleWeight(PrimarySample& sample, FLOAT w) { sample.weight = w; }
        Color4f& GetSamplePixelColor(PrimarySample& sample) { return sample.pixelColor; }
        
        RenderBucket*	    m_bucket;
        FLOAT               m_pixelSize;
    };
}
