#pragma once
/**
 * This is a class that will generate samples to adaptively super sample
 * the pixel.
 */
#include "ApolloCommon.h"
#include "PrimarySampler.h"

namespace Apollo {
    class RenderBucket;
    class AdaptiveSuperSampler : public PrimarySampler {
	public:
	    struct Settings {
	        Settings() {
		        minLevels   = 1;
		        maxLevels   = 3;
		        threshold   = Color4f(.05f, .05f, .05f, .05f);
	        }

	        Settings(BYTE maxLevels_, BYTE minLevels_=1) {
		        minLevels = minLevels_;
		        maxLevels = maxLevels_;
		        threshold = Color4f(.1f, .1f, .1f, .1f);
	        }

	        Settings(BYTE minLevels_, BYTE maxLevels_, const Color4f& threshold_) {
		        minLevels = minLevels_;
		        maxLevels = maxLevels_;
		        threshold = threshold_;
	        }

	        BYTE	    minLevels;
	        BYTE	    maxLevels;
	        Color4f     threshold;
        };


	public:
        AdaptiveSuperSampler();
	    AdaptiveSuperSampler(Settings& settings);

	    virtual ~AdaptiveSuperSampler();
	    virtual bool GetNextSample(PrimarySample& sample);

    private:
        struct AdaptiveSampleNode {
	        FLOAT	            x;
	        FLOAT	            y;
	        FLOAT	            weight;
	        BYTE	            depth;
	        Color4f	            color;
            const Model*        model;
	    };

    private:
        void AddFourSamples(AdaptiveSampleNode& node, const Color4f& c, const Model*);
        bool IsColorDifferenceAboveThreshold(const Color4f&, const Color4f&) const;
        void Init();

	private:
	    AdaptiveSampleNode*	    m_stack;
	    UINT32		            m_size;
        UINT32                  m_maxNodes;
	    Color4f		            m_color;
	    UINT32			        m_row;
	    UINT32			        m_col;
        bool                    m_firstSample;
        bool                    m_commonCase;
	    Settings		        m_settings;
    };

};

