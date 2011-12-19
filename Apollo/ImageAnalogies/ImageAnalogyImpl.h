#pragma once
/**
 * Internal implementation header
 */

#include "ApolloCommon.h"
#include "Image.h"
#include "ImageAnalogy.h"

class ANNkd_tree;

namespace Apollo
{

class FeaturePyramid 
{
public:
	static FeaturePyramid* CreateFeaturePyramid(GaussianPyramid* A, GaussianPyramid* Aprime, ImageAnalogy::FEATURE_TYPE);
	~FeaturePyramid();

	double* getFeatures(UINT level, UINT& length) const 
    {
		assert (level >= 0 && level < m_levels);
        length = m_numFeatures[level];
		return m_features[level];
	}

	GaussianPyramid* getImagePyramid() const {
		return m_pyramid1;
	}

	GaussianPyramid* getImagePyramidPrime() const {
		return m_pyramid2;
	}

	int getFeatureLength() const {
		return m_featureLength;
	}

	double* getFeature(UINT level, int index) const {
		assert (level >= 0 && level < m_levels);
		assert (index >= 0 && index < (int)getMaxIndex(level));
		return &m_features[level][index*m_featureLength];
	}

    UINT getMaxIndex(int level) const
    {
        return m_numFeatures[level];
    }

	static const double* getFeatureWeights() 
    {
		return m_featureWeights;
	}

	/* Computes a single feature vector centered at (row, col)
	 *    - A1: The lower resolution src image
	 *    - A2: The higher resolution src image
	 *    - B1: The lower resolution filtered image
	 *    - B2: The higher resolution filtred image
	 *
	 *   You can pass nullptr for A1 and B1 if there is no lower resolution image
	 *   These images should already be in luminous space.
	 *
	 *   Returns the length of the vector assigned.
	 *   Res should be a preallocated buffer where the feature vector will go.
	 */
	static int computeFeature(const Image* A1, const Image* A2, const Image* B1, const Image* B2, int row, int col, double* res, ImageAnalogy::FEATURE_TYPE type);

private:
	double**			m_features;
	int*				m_widths;
	int*				m_heights;
	GaussianPyramid*	m_pyramid1;
	GaussianPyramid*	m_pyramid2;
	UINT				m_levels;
	float				m_error;
	int					m_featureLength;
    std::vector<UINT>   m_numFeatures;

	void computeFeatures(GaussianPyramid* A, GaussianPyramid* Aprime, ImageAnalogy::FEATURE_TYPE type);
	
	static int computeLuminanceFeature(const Image* A1, const Image* A2, const Image* B1, const Image* B2, int row, int col, double* res);
	static int computeRGBFeature(const Image* A1, const Image* A2, const Image* B1, const Image* B2, int row, int col, double* res);

	static double* m_featureWeights;

};

struct ImageAnalogyData;

class CoherenceSearch {
public:
	/* int is the index into data->srcFeatures that produced the best distance.
	 * The best distance will be placed in distance on return.
	 */
	static int findBestMatch(ImageAnalogyData* data, int row, int col, double* target, double& distance);
};

class ANNSearch {
public:
	
	/* Create the ANN structure with the src data, the length of the
	 * data and the number of dimensions
	 * The length of the data should be the number of different features
	 * (not bytes).
	 *
	 * The data should not be deallocated or rearranged after creating
	 * this structure;
	 */
	ANNSearch(double* data, int nElements, int nDimensions, float error);
	
	/* This will return the index of the element which contains a neighbor
	 * within the error distance 
	 * The target should be a feature vector with length equal to the number
	 * of dimensions.
	 *
	 * The best distance will be stored in distance.
	 */
	int findANN(double* target, double& distance);

	/* This will return the best match (as if annError was 0)
	 */
	int findNN(double* target);

	~ANNSearch(void);

	double* getData() const {
		return m_data;
	}

	int getNumElements() const {
		return m_elements;
	}

	int getNumDimensions() const {
		return m_dimensions;
	}

	float getError() const {
		return m_error;
	}

private:
	double*			m_data;
	int				m_elements;
	int				m_dimensions;
	float			m_error;
	ANNkd_tree*		m_tree;

	/* Initializes the underlying ann search structure */
	void init();
};

};
