#pragma once

#include "ApolloCommon.h"
#include "Image.h"

namespace Apollo
{
class ANNSearch;
class FeaturePyramid;

struct ImageAnalogyData 
{
	const Image*		AHigh;
	const Image*		ALow;
	const Image*		APHigh;
	const Image*		APLow;
	const Image*		BHigh;
	const Image*		BLow;
	Image*				BPHigh;
	Image*				BPLow;
	FeaturePyramid*		srcFeatures;
	ANNSearch*			annSearch;
	std::vector<int>	mapping;
	UINT				level;
};

struct ImageAnalogyDataPatchMatch
{
    const Image*		AHigh;
	const Image*		ALow;
	const Image*		APHigh;
	const Image*		APLow;
	const Image*		BHigh;
	const Image*		BLow;
	Image*				BPHigh;
	const Image*		BPLow;
    std::vector<int>    mapping;
    UINT                level;
};

class ImageAnalogy 
{
public:   
    enum FEATURE_TYPE 
    {
	    FEATURE_LUMINANCE = 0,
	    FEATURE_RGB
    };

    enum POSTPROCESS_TYPE 
    {
	    POSTPROCESS_APRIME_COLORS = 0,			//Get colors from Aprime
	    POSTPROCESS_B_COLORS					//Get colors from B
    };

public:
	ImageAnalogy(void);
	~ImageAnalogy(void);

	/* Change various settings for image analogies 
	 *   - must be set before call to createAnalogy to take effect
	 */
	void setPyramidLevels(int levels);
	void setLuminanceRemapping(bool mapping);
	void setFeatureType(FEATURE_TYPE type);
	void setANNError(float error);
	void setSaveIntermediate(bool value);
	void setCoherence(float coherence);
	void setPostProcess(POSTPROCESS_TYPE type);

	Image* createAnalogy(const Image* A, const Image* Aprime, const Image* B);
    Image* createAnalogyPatchMatch(const Image* A, const Image* Aprime, const Image* B);
	
	/* Computes norm L2 distance */
	static double computeFeatureDistance(double* f1, double* f2, int nDims);
    static float computeFeatureDistance(const std::vector<float>&, const std::vector<float>&);
	static void normalizeFeature(double* f, int nDims);

	static Image* combineImages(const Image* A, const Image* APrime, const Image* B, const Image* BPrime, int border = 10);

private:
	UINT				m_levels;
	bool				m_luminanceRemapping;
	FEATURE_TYPE		m_featureType;
	POSTPROCESS_TYPE	m_postProcessType;
	float				m_error;
	float				m_coherence;
	bool				m_saveIntermediate;

	/**
	 * Constructs one level of the image.  
	 *     - resHigh is the location where the resultant image will be placed 
	 *     - resLow is the constructed image at the previous resolution.
	 *     - srcHigh is the information from the original unfiltered target image
	 *     - srcLow is the low resolution of above
	 *     - sHigh is the resultant target to source index mapping
	 *     - sLow is the target to source mapping from the previous resolution
	 *     - annSearch is the initialize ANNSearch structure initialized with the source images
	 */
	void constructTargetImage(ImageAnalogyData* data, TaskData* task);
    void constructTargetImagePatchMatch(ImageAnalogyDataPatchMatch* data, TaskData* task);

	/* Perform final color remappings */
	void postProcess(const Image* A, const Image* APrime, const Image* B, Image* Bprime);

	/* Remap luminance channel only */
	static void postProcessLuminance(const Image* src, Image* result);

	/* Remap RGB channels */
	static void postProcessRGB(const Image* src, Image* result);

	/* Does luminance remapping - A should be the modified image */
	static void remapLuminance(Image* result, const Image* src);

    /* Constructs a patch feature vector at (row, col) in AHigh pulling in values from the lower mip Alow */
    void ConstructPatchFeatureVector(const Image* high, const Image* low, UINT row, UINT col, std::vector<float>& result);
};

};
