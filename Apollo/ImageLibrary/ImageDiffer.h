#pragma once

#include "Image.h"

namespace Apollo {
    class ImageDiffer {
    public:
        enum ImageDiffMode {
            LUMINANCE_PERCENT,
        };
    public:
	    ImageDiffer();
	    virtual ~ImageDiffer();

        // Returns true if they are different and false if identical (after 
        // applying threshold and magnify)
	    virtual bool Diff(const Image& ref, const Image& target);
        
        virtual void SetDiffMode(ImageDiffMode);

	    // Magnifies differences for display and thresholding
	    void SetMagnification(FLOAT m);

        // Sets the threshold
        void SetThreshold(FLOAT t);

        virtual const Image* GetDiffData();

    protected:
        void ComputePercentDiff();
        void UpdateDisplay();
        
    protected:
        const Image*    m_ref;
        const Image*    m_target;
        Image*          m_diffData;
        Image*          m_displayData;
        bool            m_displayDirty;
	    ImageDiffMode   m_mode;
	    FLOAT           m_magnify;
	    FLOAT           m_threshold;

	    // Compute display types
	    void computeABSDifference();
	    void computeErrorColorDifference(const Color4f& c);
    };
}
