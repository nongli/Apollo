#include "TextureSynthesis.h"
#include "Image.h"

namespace Apollo
{
    TextureSynthesis::TextureSynthesis(void) 
    {
    }

    TextureSynthesis::~TextureSynthesis(void) 
    {
    }

    Image* TextureSynthesis::SynthesizeTexture(const Image* source, Image* dst, const TextureSynthesisParams& params) 
    {
	    TextureSynthesis ts;
        ts.m_result = dst;
	    ts.m_params = params;
	    ts.m_source = source;

        //for (int i = 0; i < 50; i++)
        for (int i = 0; i < 10; i++)
	    ts.createTexture();
	    return ts.m_result;
    }

    void TextureSynthesis::createTexture() 
    {
	    init();

        int w = m_result->GetWidth();
        int h = m_result->GetHeight();

	    int bestCol = 0;
        int bestRow = 0;
        FLOAT bestDiff;

	    /* Generate the main portion of the image */
	    for (int row = 0; row < h - m_params.regionHeight / 2; row++) 
        {
		    for (int col = 0; col < w; col++) 
            {
			    int nCand = generateCandidates(row, col);
		
			    /* If the number of neighbors is greater than 1, find the L2 distance
			     * for every candidate and choose the best one */
			    if (nCand > 1) 
                {
				    bestDiff = 1e10;
				    for (int k = 0; k < nCand; k++) 
                    {
					    FLOAT diff = neighborDiff(m_candidates[k], row, col);
					    if (diff < bestDiff) 
                        {
						    bestDiff = diff;
						    bestCol = m_candidates[k].col;
						    bestRow = m_candidates[k].row;
					    }
				    }
			    }
			    /* Otherwise, pick the first one */
			    else 
                {
				    bestCol = m_candidates[0].col;
				    bestRow = m_candidates[0].row;
			    }

			    /* Copy the best pixel into result and atlas */
			    Color4f pixel = m_source->GetPixel(bestRow, bestCol);
                m_result->SetPixel(pixel, row, col);
			    m_atlas[row*w + col].row = bestRow;
			    m_atlas[row*w + col].col = bestCol;
		    }
	    }

	    /* Special case last few rows */
	    for (int row = h - m_params.regionHeight/2; row < h; row++) 
        {
		    for (int col = 0; col < w; col++) 
            {
			    int nCand = generateAllCandidates(row, col);

			    /* If the number of neighbors is greater than 1, find the L2 distance
			     * for every candidate and choose the best one */
			    if (nCand > 1) 
                {
				    bestDiff = 1e10;
				    for (int k = 0; k < nCand; k++) 
                    {
					    FLOAT diff = neighborAllDiff(m_candidates[k], row, col);
					    if (diff < bestDiff) 
                        {
						    bestDiff = diff;
						    bestCol = m_candidates[k].col;
						    bestRow = m_candidates[k].row;
					    }
				    }
			    }
			    /* Otherwise, pick the first one */
			    else 
                {
				    bestCol = m_candidates[0].col;
				    bestRow = m_candidates[0].row;
			    }

			    /* Copy the best pixel into result and atlas */
			    Color4f pixel = m_source->GetPixel(bestRow, bestCol);
			    m_result->SetPixel(pixel, row, col);
                m_atlas[row*w + col].row = bestRow;
			    m_atlas[row*w + col].col = bestCol;
		    }
	    }

	    /* Redo the top of the image */
	    for (int row = 0; row < m_params.regionHeight/2; row++) 
        {
		    for (int col = 0; col < w; col++) 
            {
			    int nCand = generateAllCandidates(row, col);

			    /* If the number of neighbors is greater than 1, find the L2 distance
			     * for every candidate and choose the best one */
			    if (nCand > 1) 
                {
				    bestDiff = 1e10;
				    for (int k = 0; k < nCand; k++) 
                    {
					    FLOAT diff = neighborAllDiff(m_candidates[k], row, col);
					    if (diff < bestDiff) {
						    bestDiff = diff;
						    bestCol = m_candidates[k].col;
						    bestRow = m_candidates[k].row;
					    }
				    }
			    }
			    /* Otherwise, pick the first one */
			    else 
                {
				    bestCol = m_candidates[0].col;
				    bestRow = m_candidates[0].row;
			    }

			    /* Copy the best pixel into result and atlas */
			    Color4f pixel = m_source->GetPixel(bestRow, bestCol);
			    m_result->SetPixel(pixel, row, col);
			    m_atlas[row*w + col].row = bestRow;
			    m_atlas[row*w + col].col = bestCol;
		    }
	    }	
    }

    /* Initializes data for texture synthesis:
     *  - Creates result image
     *  - Allocates the atlas
     *	- Allocates candidate array 
     *  - Initializes result image to random values
     */
    void TextureSynthesis::init() 
    {
        UINT w = m_result->GetWidth();
        UINT h = m_result->GetHeight();

        m_candidates.resize(m_params.regionWidth * m_params.regionHeight + 1);
        m_atlas.resize(w * h);
        
	    m_vrstartx	= m_params.regionWidth / 2;
	    m_vrstarty	= m_params.regionHeight / 2;
	    m_vrfinishx = m_source->GetWidth() - m_vrstartx;
	    m_vrfinishy = m_source->GetHeight() - m_vrstarty;
	
	    UINT tmpX, tmpY;
	    int xRange = m_vrfinishx - m_vrstartx;
	    int yRange = m_vrfinishy - m_vrstarty;

	    for (UINT row = 0; row < h; row++) 
        {
		    for (UINT col = 0; col < w; col++) 
            {
			    tmpX = m_vrstartx + rand()%xRange;
			    tmpY = m_vrstarty + rand()%yRange;

			    m_atlas[row*w + col].col = tmpX;
			    m_atlas[row*w + col].row = tmpY;

			    Color4f pixel = m_source->GetPixel(tmpY, tmpX);
			    m_result->SetPixel(pixel, row, col);
		    }
	    }
    }

    /**
     *  Generates causal neighbor candidates.
     */
    int TextureSynthesis::generateCandidates(int row, int col) 
    {
	    int numNeighbors = 0;
	    int xRange = m_vrfinishx - m_vrstartx;
	    int yRange = m_vrfinishy - m_vrstarty;

        UINT w = m_result->GetWidth();
        UINT h = m_result->GetHeight();

	    for (int r = -m_params.regionHeight / 2; r <= 0; r++) 
        {
		    for (int c = -m_params.regionWidth/2; c <= m_params.regionWidth/2; c++) 
            {
			    /* Skip if not in L-shape */
			    if (r == 0 && c >= 0)
				    continue;
			
			    int rIndex = r + row;
			    int cIndex = c + col;

			    rIndex = (rIndex + h) % h;
			    cIndex = (cIndex + w) % w;

			    Index atlasIndex = m_atlas[rIndex * w + cIndex];
                m_candidates[numNeighbors].row = atlasIndex.row - r;
			    m_candidates[numNeighbors].col = atlasIndex.col - c;

			    /* Randomize borders */
			    if (m_candidates[numNeighbors].col >= m_vrfinishx || 
						    m_candidates[numNeighbors].col < m_vrstartx ||
						    m_candidates[numNeighbors].row >= m_vrfinishy) 
                {
				    m_candidates[numNeighbors].col = m_vrstartx + rand()%xRange;
				    m_candidates[numNeighbors].row = m_vrstarty + rand()%yRange;
				    numNeighbors++;
				    continue;
			    }

			    /* Remove duplicates */		
			    for (int k = 0; k < numNeighbors; k++) 
                {
				    if (m_candidates[k].row == m_candidates[numNeighbors].row &&
					    m_candidates[k].col == m_candidates[numNeighbors].col) 
                    {
						numNeighbors--;
						break;
				    }
			    }

			    numNeighbors++;
		    }
	    }

	    return numNeighbors;
    }

    int TextureSynthesis::generateAllCandidates(int row, int col) 
    {
	    int numNeighbors = 0;
	    int xRange = m_vrfinishx - m_vrstartx;
	    int yRange = m_vrfinishy - m_vrstarty;

        UINT w = m_result->GetWidth();
        UINT h = m_result->GetHeight();

	    for (int r = -m_params.regionHeight / 2; r <= m_params.regionHeight / 2; r++) 
        {
		    for (int c = -m_params.regionWidth/2; c <= m_params.regionWidth/2; c++) 
            {			
			    int rIndex = r + row;
			    int cIndex = c + col;

			    rIndex = (rIndex + h) % h;
			    cIndex = (cIndex + w) % w;

			    Index atlasIndex = m_atlas[rIndex*w + cIndex];
			    m_candidates[numNeighbors].row = atlasIndex.row - r;
			    m_candidates[numNeighbors].col = atlasIndex.col - c;

			    /* Randomize borders */
			    if (m_candidates[numNeighbors].col >= m_vrfinishx || 
						    m_candidates[numNeighbors].col < m_vrstartx ||
						    m_candidates[numNeighbors].row >= m_vrfinishy ||
						    m_candidates[numNeighbors].row < m_vrstarty) {
				    m_candidates[numNeighbors].col = m_vrstartx + rand()%xRange;
				    m_candidates[numNeighbors].row = m_vrstarty + rand()%yRange;
				    numNeighbors++;
				    continue;
			    }

			    /* Remove duplicates */	
			    for (int k = 0; k < numNeighbors; k++) 
                {
				    if (m_candidates[k].row == m_candidates[numNeighbors].row &&
					    m_candidates[k].col == m_candidates[numNeighbors].col) 
                    {
						    numNeighbors--;
						    break;
				    }
			    }

			    numNeighbors++;
		    }
	    }

	    return numNeighbors;
    }

    /* Computes the L2 neighborhood distance */
    FLOAT TextureSynthesis::neighborDiff(const Index& index, int row, int col) 
    {
        UINT w = m_result->GetWidth();
        UINT h = m_result->GetHeight();

        UINT rowOff = row + h;
        UINT colOff = col + w;

	    FLOAT distance = 0;

	    for (int r = -m_params.regionHeight/2; r <= 0; r++) 
        {
		    for (int c = -(m_params.regionWidth/2); c <= m_params.regionWidth/2; c++) 
            {
			    if (r == 0 && c >= 0)
				    continue;

			    int srcRow = index.row + r;
			    int srcCol = index.col + c;
			    int destRow = (r + rowOff) % h;
			    int destCol = (c + colOff) % w;

			    Color4f srcPixel = m_source->GetPixel(srcRow, srcCol);
			    Color4f destPixel = m_result->GetPixel(destRow, destCol);

                distance += srcPixel.Distance2RGB(destPixel);
		    }
	    }
	    return distance;
    }

    FLOAT TextureSynthesis::neighborAllDiff(const Index& index, int row, int col) 
    {
        UINT w = m_result->GetWidth();
        UINT h = m_result->GetHeight();

        UINT rowOff = row + h;
        UINT colOff = col + w;

	    FLOAT distance = 0;

	    for (int r = -m_params.regionHeight/2; r <= m_params.regionHeight/2; r++) 
        {
		    for (int c = -(m_params.regionWidth/2); c <= m_params.regionWidth/2; c++) 
            {

			    if (r > 0 && r > m_params.regionHeight && row + r < m_params.regionHeight)
				    continue;

			    int srcRow = index.row + r;
			    int srcCol = index.col + c;
			    int destRow = (r + rowOff) % h;
			    int destCol = (c + colOff) % w;

			    Color4f srcPixel = m_source->GetPixel(srcRow, srcCol);
			    Color4f destPixel = m_result->GetPixel(destRow, destCol);

                distance += srcPixel.Distance2RGB(destPixel);
		    }
	    }
	    return distance;
    }
}