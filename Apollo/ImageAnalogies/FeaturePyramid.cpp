#include "ImageAnalogyImpl.h"

namespace Apollo
{

enum BORDER_TYPE {
    BORDER_REPEAT,
    BORDER_REFLECT
};

#define RESOLVE_BORDER_REFLECT(index, max)      \
    if (index < 0)                              \
    {                                           \
        index = -index;                         \
        index = index % ((max) + 1);            \
    }                                           \
    else if (index > (max) )                    \
    {                                           \
        while (index > 2*((max) + 1))		    \
			index -= (max + 1);			        \
		index = (max + 1)*2 - index;	        \
		index = index % ((max) + 1);	        \
    }                                           \


/*
#define RESOLVE_BORDER(index, max, type)			\
	do {											\
		if (index < 0) {							\
			switch(type) {							\
				case BORDER_REPEAT:					\
					index = 0;						\
					break;							\
				case BORDER_REFLECT:				\
					index = -index;					\
					index = index % ((max) + 1);	\
					break;							\
				default:							\
					index = 0;						\
					break;							\
			}										\
		}											\
		else if (index > (max)) {					\
			switch(type) {							\
				case BORDER_REPEAT:					\
					index = (max);					\
					break;							\
				case BORDER_REFLECT:				\
					while (index > 2*(max + 1))		\
						index -= (max + 1);			\
					index = (max + 1)*2 - index;	\
					index = index % ((max) + 1);	\
					break;							\
				default:							\
					index = (max);					\
					break;							\
			}										\
		}											\
	} while(0)
*/

double* FeaturePyramid::m_featureWeights;

FeaturePyramid* FeaturePyramid::CreateFeaturePyramid(
        GaussianPyramid* A, 
        GaussianPyramid* Aprime, 
        ImageAnalogy::FEATURE_TYPE type) 
{
	FeaturePyramid* result = new FeaturePyramid();
	
	/* Initialize memory for result */
	result->m_pyramid1 = A;
	result->m_pyramid2 = Aprime;
	result->m_levels = A->GetNumLevels();
	result->m_widths = new int[result->m_levels];
	result->m_heights = new int[result->m_levels];
	result->m_features = new double*[result->m_levels];
	result->m_featureWeights = new double[55];

	double sigma = 0.6;
	double maxDistance = sqrt(8.0);

	/* Initialize lower level weights */
	double sum = 0.0;
	for (int i = 0; i < 9; i++) {
		int rDist = i/3 - 1;
		int cDist = i%3 - 1;
		double d = sqrt(rDist*rDist + cDist*cDist + 1.0) / maxDistance;
		d = -d / (2*sigma*sigma);
		double weight = pow(APOLLO_E, d);
		sum += (weight + weight);
		result->m_featureWeights[i] = weight;
		result->m_featureWeights[i + 34] = weight;
	}
	for (int i = 0; i < 9; i++) {
		result->m_featureWeights[i] /= sum;
		result->m_featureWeights[i + 34] /= sum;
	}

	/* Initialize the upper level */
	sum = 0.0;
	for (int i = 0; i < 25; i++) {
		int rDist = i/5 - 2;
		int cDist = i%5 - 2;
		double d = sqrt((double)rDist*rDist + (double)cDist*cDist) / maxDistance;
		d = -d / (2*sigma*sigma);
		double weight = pow(APOLLO_E, d);
		sum += weight;
		result->m_featureWeights[i + 9] = weight;
	}
	for (int i = 0; i < 12; i++) {
		int rDist = i/5 - 2;
		int cDist = i%5 - 2;
		double d = sqrt((double)rDist*rDist + (double)cDist*cDist) / maxDistance;
		d = -d / (2*sigma*sigma);
		double weight = pow(APOLLO_E, d);
		sum += weight;
		result->m_featureWeights[i + 43] = weight;
	}
	for (int i = 0; i < 25; i++) {
		result->m_featureWeights[i + 9] /= sum;
	}
	for (int i = 0; i < 12; i++) {
		result->m_featureWeights[i + 43] /= sum;
	}

	/* Compute feature data differently for different feature types */
	result->computeFeatures(A, Aprime, type);

	return result;
}

FeaturePyramid::~FeaturePyramid() {
	delete[] m_widths;
	delete[] m_heights;
	for (UINT i = 0; i < m_levels; i++) {
		delete[] m_features[i];
	}
	delete[] m_features;
}

int FeaturePyramid::computeFeature(
            const Image* A1, 
            const Image* A2, 
            const Image* B1, 
            const Image* B2, 
            int row, int col, 
            double* res, 
            ImageAnalogy::FEATURE_TYPE type) 
{
	switch (type) {
		case ImageAnalogy::FEATURE_LUMINANCE:
			return computeLuminanceFeature(A1, A2, B1, B2, row, col, res);
		case ImageAnalogy::FEATURE_RGB:
			return computeRGBFeature(A1, A2, B1, B2, row, col, res);
	}
	return 0;
}


/* The length of the vector is 55.
 *       - The source images have complete information 
 *       -    9 elements come from A1
 *       -   25 elements come from A2
 *       - The filtered image is assumed to be incomplete (for B')
 *       -    9 elements come from B1
 *       -   12 elements come from B2
 *
 * The layout of the resulting vector will be:
 *       A1, A2, B1, B2
 *
 * TODO: I am currently having all features be the same length (55)
 *       and NOT special casing the lowest resolution image.  It has
 *       zeroes where the data is missing.  Does this mess up the
 *       ann search?
 *
 * TODO: Right now it is using the suggest 5x5 and 3x3 for the neighborhoods
 *       This should be configurable.
 */
int FeaturePyramid::computeLuminanceFeature(
                const Image* A1, 
                const Image* A2, 
                const Image* B1, 
                const Image* B2, 
                int row, int col, 
                double* res) 
{

	int rowIndex;
	int colIndex;
	int width = 0;
	int height = 0;
	int length = 0;
                                    
	/* Fill in A1 */
	if (A1 != nullptr) 
    {
		width = A1->GetWidth();
		height = A1->GetHeight();
	}
	for (int r = -1; r < 2; r++) 
    {
		for (int c = -1; c < 2; c++) 
        {
			if (A1 != nullptr) 
            {
				rowIndex = r + row/2;
				colIndex = c + col/2;
				RESOLVE_BORDER_REFLECT(rowIndex, height - 1);
				RESOLVE_BORDER_REFLECT(colIndex, width - 1);
                (*res++) = A1->GetPixel(rowIndex, colIndex).ToLuminance();
			}
			else 
            {
				(*res++) = 0;
			}
			length++;
		}
	}

	/* Fill in A2 */
	width = A2->GetWidth();
	height = A2->GetHeight();
	for (int r = -2; r < 3; r++) {
		for (int c = -2; c < 3; c++) {
			rowIndex = r + row;
			colIndex = c + col;
			RESOLVE_BORDER_REFLECT(rowIndex, height - 1);
			RESOLVE_BORDER_REFLECT(colIndex, width - 1);
			(*res++) = A2->GetPixel(rowIndex, colIndex).ToLuminance();
			length++;
		}
	}

	/* Fill in B1 */
	if (B1 != nullptr) {
		width = B1->GetWidth();
		height = B1->GetHeight();
	}
	for (int r = -1; r < 2; r++) {
		for (int c = -1; c < 2; c++) {
			if (B1 != nullptr) {
				rowIndex = r + row/2;
				colIndex = c + col/2;
				RESOLVE_BORDER_REFLECT(rowIndex, height - 1);
				RESOLVE_BORDER_REFLECT(colIndex, width - 1);
				(*res++) = B1->GetPixel(rowIndex, colIndex).ToLuminance();
			}
			else {
				(*res++) = 0;
			}
			length++;
		}
	}

	/* Fill in portion of B2 */
	width = B2->GetWidth();
	height = B2->GetHeight();
	for (int r = -2; r <= 0; r++) {
		for (int c = -2; c < 3; c++) {
			if (r == 0 && c >= 0)
				break;
			rowIndex = r + row;
			colIndex = c + col;
			RESOLVE_BORDER_REFLECT(rowIndex, height - 1);
			RESOLVE_BORDER_REFLECT(colIndex, width - 1);
			(*res++) = B2->GetPixel(rowIndex, colIndex).ToLuminance();
			length++;
		}
	}

	/* Scale feature */
	const double* weights = FeaturePyramid::getFeatureWeights();
	for (int i = 0; i < length; i++) 
    {
		(res-length)[i] *= weights[i];
	}

	return length;
}

int FeaturePyramid::computeRGBFeature(
                const Image* A1, 
                const Image* A2, 
                const Image* B1, 
                const Image* B2, 
                int row, int col, 
                double* res) {

	int rowIndex;
	int colIndex;
	int width;
	int height;
	int length = 0;
                                    
	/* Fill in A1 */
	if (A1 != nullptr) {
		width = A1->GetWidth();
		height = A1->GetHeight();
	}
	for (int r = -1; r < 2; r++) {
		for (int c = -1; c < 2; c++) {
			if (A1 != nullptr) {
				rowIndex = r + row/2;
				colIndex = c + col/2;
				RESOLVE_BORDER_REFLECT(rowIndex, height - 1);
				RESOLVE_BORDER_REFLECT(colIndex, width - 1);
                const Color4f& pixel = A1->GetPixel(rowIndex, colIndex);
                (*res++) = pixel.r;
                (*res++) = pixel.g;
                (*res++) = pixel.b;
			}
			else {
				(*res++) = 0;
				(*res++) = 0;
				(*res++) = 0;
			}
			length+=3;
		}
	}

	/* Fill in A2 */
	width = A2->GetWidth();
	height = A2->GetHeight();
	for (int r = -2; r < 3; r++) {
		for (int c = -2; c < 3; c++) {
			rowIndex = r + row;
			colIndex = c + col;
			RESOLVE_BORDER_REFLECT(rowIndex, height - 1);
			RESOLVE_BORDER_REFLECT(colIndex, width - 1);
            const Color4f& pixel = A2->GetPixel(rowIndex, colIndex);
            (*res++) = pixel.r;
            (*res++) = pixel.g;
            (*res++) = pixel.b;
			length+=3;
		}
	}

	/* Fill in B1 */
	if (B1 != nullptr) {
		width = B1->GetWidth();
		height = B1->GetHeight();
	}
	for (int r = -1; r < 2; r++) {
		for (int c = -1; c < 2; c++) {
			if (B1 != nullptr) {
				rowIndex = r + row/2;
				colIndex = c + col/2;
				RESOLVE_BORDER_REFLECT(rowIndex, height - 1);
				RESOLVE_BORDER_REFLECT(colIndex, width - 1);
                const Color4f& pixel = B1->GetPixel(rowIndex, colIndex);
                (*res++) = pixel.r;
                (*res++) = pixel.g;
                (*res++) = pixel.b;
			}
			else {
				(*res++) = 0;
				(*res++) = 0;
				(*res++) = 0;
			}
			length+=3;
		}
	}

	/* Fill in portion of B2 */
	width = B2->GetWidth();
	height = B2->GetHeight();
	for (int r = -2; r <= 0; r++) {
		for (int c = -2; c < 3; c++) {
			if (r == 0 && c >= 0)
				break;
			rowIndex = r + row;
			colIndex = c + col;
			RESOLVE_BORDER_REFLECT(rowIndex, height - 1);
			RESOLVE_BORDER_REFLECT(colIndex, width - 1);
            const Color4f& pixel = B2->GetPixel(rowIndex, colIndex);
            (*res++) = pixel.r;
            (*res++) = pixel.g;
            (*res++) = pixel.b;
			length+=3;
		}
	}

	/* Scale feature */
	const double* weights = FeaturePyramid::getFeatureWeights();
	for (int i = 0; i < length; i++) {
		(res-length)[i] *= weights[i/3];
	}

	return length;
}

/* Computes all of the luminance features for the entire image */
void FeaturePyramid::computeFeatures(
                GaussianPyramid* A, 
                GaussianPyramid* B, 
                ImageAnalogy::FEATURE_TYPE type) {

	const Image* Alow, *Ahigh;
	const Image* Blow, *Bhigh;

	Alow = Blow = nullptr;

	switch (type) {
		case ImageAnalogy::FEATURE_LUMINANCE:
			m_featureLength = 55;
			break;
		case ImageAnalogy::FEATURE_RGB:
			m_featureLength = 165;
			break;
		default:
			m_featureLength = 55;
			type = ImageAnalogy::FEATURE_LUMINANCE;
			break;
	}
	
    m_numFeatures.resize(m_levels);

	for (int level = m_levels - 1; level >= 0; level--) {
		Ahigh = A->GetImageAt(level);
		Bhigh = B->GetImageAt(level);

		int nFeatures = Ahigh->GetSize();
		m_widths[level] = Ahigh->GetWidth();
		m_heights[level] = Ahigh->GetHeight();
		m_features[level] = new double[nFeatures*m_featureLength];
		memset(m_features[level], 0, sizeof(double)*nFeatures*m_featureLength);
		double* buffer = m_features[level];

		/* Loop through entire image and compute the feature vector for each index */
		for (UINT row = 0; row < Ahigh->GetHeight(); row++) 
        {
			for (UINT col = 0; col < Ahigh->GetWidth(); col++) 
            {
				int length = computeFeature(Alow, Ahigh, Blow, Bhigh, row, col, buffer, type);
                buffer += length;
            }
		}

        m_numFeatures[level] = nFeatures;

		Alow = Ahigh;
		Blow = Bhigh;
	}
}

}
