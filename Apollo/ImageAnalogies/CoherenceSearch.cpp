#include "ImageAnalogyImpl.h"
#include "ImageAnalogy.h"

namespace Apollo
{

int generateCandidates(ImageAnalogyData* data, int row, int col, int* candidates) 
{
	int numNeighbors = 0;

	int width = data->BPHigh->GetWidth();
	int height = data->BPHigh->GetHeight();

	for (int r = -2; r <= 0; r++) 
    {
		for (int c = -2; c <= 2; c++) 
        {
			/* Skip if not in L-shape */
			if (r == 0 && c >= 0)
				break;
			
			int rIndex = r + row;
			int cIndex = c + col;

			rIndex = (rIndex + height) % height;
			cIndex = (cIndex + width) % width;

			if (data->mapping[rIndex*width + cIndex] < 0)
				continue;

			int atlasRow = data->mapping[rIndex*width + cIndex] / data->APHigh->GetWidth();
			int atlasCol = data->mapping[rIndex*width + cIndex] % data->APHigh->GetWidth();

			int index = (atlasRow - r)*data->APHigh->GetWidth() + atlasCol - c;
			/* Ignore if out of bounds */
			if (index < 0 || (UINT)index >= data->srcFeatures->getMaxIndex(data->level)) {
				continue;
			}
			candidates[numNeighbors] = index;

			numNeighbors++;
		}
	}

	return numNeighbors;
}

int CoherenceSearch::findBestMatch(ImageAnalogyData* data, int row, int col, double* searchFeature, double& bestDistance) 
{
	int nDims = data->srcFeatures->getFeatureLength();
	int bestIndex = 0;
	double dist;
	double* srcFeature;
	bestDistance = 1e10;

	int candidates[12];

	/* Generate the possible candidates */
	int nCand = generateCandidates(data, row, col, candidates);

	if (nCand != 12) {
		return bestIndex;
	}

	/* Compute distance to every candidate */
	for (int k = 0; k < nCand; k++) {
		int index = candidates[k];
		
		/* Get the search feature vector from the source image */
		srcFeature = data->srcFeatures->getFeature(data->level, index);
		
		/* Compute distance from search feature to search feature */
		dist = ImageAnalogy::computeFeatureDistance(srcFeature, searchFeature, nDims);

		if (dist < bestDistance) {
			bestDistance = dist;
			bestIndex = index;
		}
	}

	return bestIndex;
}

}
