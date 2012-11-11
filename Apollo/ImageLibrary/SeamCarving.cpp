#include "SeamCarving.h"
#include <assert.h>

using namespace Apollo;
using namespace std;

static int PRECOMPUTE_MIN = 10;
static int BUCKET_SIZE = 50; 

namespace Apollo {

class SeamCarveCacheHorizontal {
public:
    SeamCarveCacheHorizontal(SimpleImage<BYTE>* originalImage);
    void AddSeam(int index, int* seam);
    void GenerateImage(int width, SimpleImage<BYTE>* image) const;
    int GetMinCacheWidth() const {
        return m_minCacheWidth;
    }

private:
    vector<int> m_seamCache;
    vector<int> m_buckets;
    int m_minCacheWidth;
    int m_width;
    int m_height;
    int m_nBuckets;
    SimpleImage<BYTE>* m_image;
};

class SeamCarveCacheVertical {
public:
    SeamCarveCacheVertical(SimpleImage<BYTE>* originalImage);
    void AddSeam(int index, int* seam);
    void GenerateImage(int width, SimpleImage<BYTE>* image) const;
    int GetMinCacheHeight() const {
        return m_minCacheHeight;
    }

private:
    vector<int> m_seamCache;
    vector<int> m_buckets;
    int m_minCacheHeight;
    int m_width;
    int m_height;
    int m_nBuckets;
    SimpleImage<BYTE>* m_image;
};
}

SeamCarveCacheHorizontal::SeamCarveCacheHorizontal(SimpleImage<BYTE>* image) {
    m_image = image;
    m_width = image->GetWidth();
    m_height = image->GetHeight();
    m_minCacheWidth = m_width;
    m_seamCache.resize(m_width * m_height);

    m_nBuckets = m_width / BUCKET_SIZE + 1;
    m_buckets.resize(m_nBuckets * m_height);
}

SeamCarveCacheVertical::SeamCarveCacheVertical(SimpleImage<BYTE>* image) {
    m_image = image;
    m_width = image->GetWidth();
    m_height = image->GetHeight();
    m_minCacheHeight = m_height;
    m_seamCache.resize(m_width * m_height);

    m_nBuckets = m_height / BUCKET_SIZE + 1;
    m_buckets.resize(m_nBuckets * m_width);
}

void SeamCarveCacheHorizontal::AddSeam(int width, int* seam) {
    if (width - 1 < m_minCacheWidth) {   
        for (int row = 0; row < m_height; row++) {
            int preComputeIndex = row * m_width;           
            int offset = seam[row];
            
            bool found = false;

            for (int iBucket = 0; iBucket < m_nBuckets && !found; iBucket++) {
                int vacant = BUCKET_SIZE - m_buckets[row * m_nBuckets + iBucket];
                if (offset >= vacant) {
                    offset -= vacant;
                } else {
                    // Somewhere in this bucket
                    preComputeIndex = iBucket * BUCKET_SIZE + row * m_width;
                    for (int col = 0; col < BUCKET_SIZE; col++,preComputeIndex++) {
                        if (m_seamCache[preComputeIndex] == 0) {
                            if (offset == 0) break;
                            offset--;
                        }
                    }
                    found = true;
                }
            }           

            int nBucket = (preComputeIndex - row*m_width) / BUCKET_SIZE;
            m_buckets[row * m_nBuckets + nBucket]++;

            m_seamCache[preComputeIndex] = width;
        }
        
        m_minCacheWidth = width - 1;
    }
}

void SeamCarveCacheVertical::AddSeam(int height, int* seam) {
    if (height - 1 < m_minCacheHeight) {
        for (int col = 0; col < m_width; col++) {
            int offset = seam[col];
            int preComputeIndex = col;
            bool found = false;

            for (int iBucket = 0; iBucket < m_nBuckets && !found; iBucket++) {
                int vacant = BUCKET_SIZE - m_buckets[col * m_nBuckets + iBucket];
                if (offset >= vacant) {
                    offset -= vacant;
                } else {
                    // Somewhere in this bucket
                    preComputeIndex = iBucket * BUCKET_SIZE * m_width + col;
                    for (int row = 0; row < BUCKET_SIZE; row++,preComputeIndex+=m_width) {
                        if (m_seamCache[preComputeIndex] == 0) {
                            if (offset == 0) break;
                            offset--;
                        }
                    }
                    found = true;
                }
            }

            int nBucket = (preComputeIndex - col) / (BUCKET_SIZE * m_width);
            m_buckets[col * m_nBuckets + nBucket]++;

            m_seamCache[preComputeIndex] = height;
        }

        m_minCacheHeight = height - 1;
    }
}

void SeamCarveCacheHorizontal::GenerateImage(int width, SimpleImage<BYTE>* image) const {   
    SimpleImage<BYTE>::Pixel* pixels = image->GetData();
    SimpleImage<BYTE>::Pixel* originalPixels = m_image->GetData();

    if (width == m_width) {
        memcpy(pixels, originalPixels, m_width * m_height * sizeof(SimpleImage<BYTE>::Pixel));
    } else {
        int precomputeIndex = 0;

        for (int row = 0, rowIndex = 0; row < m_height; row++, rowIndex += m_width) {
            int colIndex = 0;
            for (int col = 0; col < m_width; col++, precomputeIndex++) {
                if (m_seamCache[precomputeIndex] < width) {
                    pixels[rowIndex + colIndex] = originalPixels[rowIndex + col];
                    colIndex++;
                }
            }
        }
    }
}

void SeamCarveCacheVertical::GenerateImage(int height, SimpleImage<BYTE>* image) const {
    SimpleImage<BYTE>::Pixel* pixels = image->GetData();
    SimpleImage<BYTE>::Pixel* originalPixels = m_image->GetData();
    
    if (height == m_height) {
        memcpy(pixels, originalPixels, m_height*m_width * sizeof(SimpleImage<BYTE>::Pixel));
    } else {
        for (int col = 0; col < m_width; col++) {
            int rowIndex = col;
            for (int row = 0, precomputeIndex = col; row < m_height; row++, precomputeIndex += m_width) {
                if (m_seamCache[precomputeIndex] < height) {
                    pixels[rowIndex] = originalPixels[precomputeIndex];
                    rowIndex += m_width;
                }
            }
        }
    }
}

SeamCarving::SeamCarving(SimpleImage<BYTE>* image, SimpleImage<BYTE>* result, SeamCacheType type) {
    m_originalImage = image;
    m_image = result;
    m_seamCacheType = type;

    m_horizontalCache = new SeamCarveCacheHorizontal(m_originalImage);
    m_verticalCache = new SeamCarveCacheVertical(m_originalImage);
    Reset();
   
    if (m_seamCacheType == SeamCachePrecompute) {
        PrecomputeHorizontalSeams();
        Reset();
        PrecomputeVerticalSeams();
    }
}

SeamCarving::~SeamCarving() {
    if (m_horizontalCache) delete m_horizontalCache;
    if (m_verticalCache) delete m_verticalCache;
}
        
SimpleImage<BYTE>* SeamCarving::SeamCarve(int width, int height)
{
    if (width > m_originalImage->GetWidth() || height > m_originalImage->GetHeight()) return NULL;
    if (width == m_ImageWidth && height == m_ImageHeight) return m_image;

    // Don't support scaling both dimensions at once.
    if (width != m_image->GetWidth() && height != m_image->GetHeight())
    {
        if (width > m_image->GetWidth() || height > m_image->GetHeight())
        {
            return NULL;
        }

        for (int w = m_ImageWidth; w > width; w--)
        {
            ComputeVerticalDP();
            ComputeVerticalSeam();
            RemoveVerticalSeam();
        }

        for (int h = m_ImageHeight; h > height; h--)
        {
            ComputeHorizontalDP();
            ComputeHorizontalSeam();
            RemoveHorizontalSeam();
        }
        return m_image;
    }
    else
    {
        // Scaling width
        if (m_ImageWidth != width)
        {
            // Go to the cache
            if (width >= m_horizontalCache->GetMinCacheWidth())
            {
                m_horizontalCache->GenerateImage(width, m_image);
                m_ImageWidth = width;
                m_energyDirty = true;
            }
            else
            {
                assert(width < m_ImageWidth);
                for (int w = m_ImageWidth; w > width; w--)
                {
                    ComputeVerticalDP();
                    ComputeVerticalSeam();
                    if (m_seamCacheType == SeamCacheDynamic)
                    {
                        m_horizontalCache->AddSeam(m_ImageWidth, &m_seam[0]);
                    }
                    RemoveVerticalSeam();
                }
            }
        }

        if (m_ImageHeight != height)
        {
            // Go to the cache
            if (height >= m_verticalCache->GetMinCacheHeight())
            {
                m_verticalCache->GenerateImage(height, m_image);
                m_ImageHeight = height;
                m_energyDirty = true;
            }
            else
            {
                assert(height < m_ImageHeight);
                for (int h = m_ImageHeight; h > height; h--)
                {
                    ComputeHorizontalDP();
                    ComputeHorizontalSeam();
                    if (m_seamCacheType == SeamCacheDynamic)
                    {
                        m_verticalCache->AddSeam(m_ImageHeight, &m_seam[0]);
                    }
                    RemoveHorizontalSeam();
                }
            }
        }
    }

    return m_image;
}
        
void SeamCarving::ComputeLuminanceDifference(int r, int c, int w)
{    
    int index = w * r + c;
    if (r <= 0 || r >= m_ImageHeight - 1 || c <= 0 || c >= m_ImageWidth - 1)
    {
        m_luminosity[index] = 255;
    }
    else
    {    
        int cur = m_luminosity[index];
        m_energy[index] = 0;
        m_energy[index] += abs(cur - m_luminosity[index + 1]);
        m_energy[index] += abs(cur - m_luminosity[index - 1]);
        m_energy[index] += abs(cur - m_luminosity[index + w]);
        m_energy[index] += abs(cur - m_luminosity[index - w]);
    }
}

void SeamCarving::Reset() {
    m_ImageWidth = m_originalImage->GetWidth();
    m_ImageHeight = m_originalImage->GetHeight();

    m_luminosity.resize(m_ImageHeight * m_ImageWidth);
    m_energy.resize(m_ImageHeight * m_ImageWidth);

    m_dp.resize(m_ImageHeight * m_ImageWidth);
    m_seam.resize(max(m_ImageWidth, m_ImageHeight));

    memcpy(m_image->GetData(), m_originalImage->GetData(), sizeof(SimpleImage<BYTE>::Pixel) * m_ImageWidth * m_ImageHeight);

    m_lastDpRun = NONE;

    ComputeEnergy();
}

void SeamCarving::ComputeEnergy() {
    SimpleImage<BYTE>::Pixel* pixels = m_image->GetData();

    int size = m_originalImage->GetWidth() * m_originalImage->GetHeight();

    for (int i = 0; i < size; i++) {
        m_luminosity[i] = pixels[i].GetLuminance();
    }
    
    int w = m_ImageWidth;
    int h = m_ImageHeight;
    int stride = m_image->GetWidth();

    for (int r = 1; r < h - 1; r++)
    {
        for (int c = 1; c < w - 1; c++)
        {
            ComputeLuminanceDifference(r, c, stride);
        }

        m_energy[r * stride] = 256;
        m_energy[r * stride + w - 1] = 256;
    }

    for (int c = 0; c < w; c++)
    {
        m_energy[c] = 256;
        m_energy[(h - 1) * stride + c] = 256;
    }

    m_energyDirty = false;
}


void SeamCarving::PrecomputeHorizontalSeams()
{           
    int maxWidth = m_image->GetWidth();

    for (int i = maxWidth; i > PRECOMPUTE_MIN; i--)
    {
        ComputeVerticalDP();
        ComputeVerticalSeam();
        m_horizontalCache->AddSeam(m_ImageWidth, &m_seam[0]);
        RemoveVerticalSeam();
    }

    m_horizontalCache->GenerateImage(maxWidth, m_image);
    m_ImageWidth = maxWidth;
    m_energyDirty = true;
}

void SeamCarving::PrecomputeVerticalSeams()
{
    
    int maxHeight = m_image->GetHeight();

    for (int i = maxHeight; i > PRECOMPUTE_MIN; i--)
    {
        ComputeHorizontalDP();
        ComputeHorizontalSeam();
        m_verticalCache->AddSeam(m_ImageHeight, &m_seam[0]);
        RemoveHorizontalSeam();
    }
                
    m_verticalCache->GenerateImage(maxHeight, m_image);
    m_ImageHeight = maxHeight;
    m_energyDirty = true;
}

void SeamCarving::ComputeVerticalDP()
{
    int w = m_ImageWidth;
    int h = m_ImageHeight;
    int stride = m_image->GetWidth();

    if (m_energyDirty)
    {
        ComputeEnergy();
    }

    // Copy first row 
    if (m_lastDpRun != VERTICAL)
    {
        memcpy(&m_dp[0], &m_energy[0], sizeof(int) * w);
    }

    int rowBefore = 0;
    int rowCurrent = stride;

    // Compute rest of dp
    if (m_lastDpRun != VERTICAL)
    {
        for (int r = 1; r < h; r++, rowBefore += stride, rowCurrent += stride)
        {
            // Special case first and last element 
            m_dp[rowCurrent + 0] = m_energy[rowCurrent + 0] + min(m_dp[rowBefore + 0], m_dp[rowBefore + 1]);
            m_dp[rowCurrent + (w - 1)] = m_energy[rowCurrent + w - 1] + min(m_dp[rowBefore + (w - 2)], m_dp[rowBefore + (w - 1)]);

            // Compute rest of the row 
            for (int c = 1; c < w - 1; c++) {
                int min = ::min(m_dp[rowBefore + (c - 1)], m_dp[rowBefore + c]);
                min = ::min(min, m_dp[rowBefore + (c + 1)]);
                int before = m_dp[rowCurrent + c];
                m_dp[rowCurrent + c] = m_energy[rowCurrent + c] + min;
            }
        }
    } else {
        int cMin = max(1, m_seam[0] - 2);
        int cMax = min(m_seam[0] + 3, w - 1);

        for (int r = 1; r < h; r++, rowBefore += stride, rowCurrent += stride)
        {
            // Special case first and last element 
            m_dp[rowCurrent + 0] = m_energy[rowCurrent + 0] + min(m_dp[rowBefore + 0], m_dp[rowBefore + 1]);
            m_dp[rowCurrent + (w - 1)] = m_energy[rowCurrent + w - 1] + min(m_dp[rowBefore + (w - 2)], m_dp[rowBefore + (w - 1)]);

            int cMinOld = m_dp[rowCurrent + cMin];
            int cMaxOld = m_dp[rowCurrent + cMax - 1];
            
            bool isLeftMin = false;
            int cNextLeft = 0;
            {
                int cMinLeft = cMin > 1 ? m_dp[rowCurrent + cMin - 2] : INT_MAX;
                int cMinCurr = m_dp[rowCurrent + cMin - 1];

                if (cMinOld < cMinLeft && cMinOld < cMinCurr)
                {
                    isLeftMin = true;
                }
                else
                {
                    cNextLeft = min(cMinLeft, cMinCurr);
                }
            }

            bool isRightMin = false;
            int cNextRight = 0;
            {
                int cMaxCurr = m_dp[rowCurrent + cMax];                
                int cMaxRight = cMax < (w - 1) ? m_dp[rowCurrent + cMax + 1] : INT_MAX;

                if (cMaxOld < cMaxRight && cMaxOld < cMaxCurr)
                {
                    isRightMin = true;
                }
                else
                {
                    cNextRight = min(cMaxRight, cMaxCurr);
                }
            }       
            
            

            // Compute rest of the row 
            for (int c = cMin; c < cMax; c++)
            {
                int min = ::min(m_dp[rowBefore + (c - 1)], m_dp[rowBefore + c]);
                min = ::min(min, m_dp[rowBefore + (c + 1)]);
                m_dp[rowCurrent + c] = m_energy[rowCurrent + c] + min;
            }
            
            if (cMin > 1 && cMinOld != m_dp[rowCurrent + cMin] &&
                (isLeftMin || m_dp[rowCurrent + cMin] < cNextLeft))
            {
                cMin--;
            }
            
            if (cMax < w - 1 && cMaxOld != m_dp[rowCurrent + cMax - 1] &&
                (isRightMin || m_dp[rowCurrent + cMax - 1] < cNextRight))
            {
                cMax++;
            }
        }
    }

    m_lastDpRun = VERTICAL;
}

void SeamCarving::ComputeVerticalSeam()
{
    int w = m_ImageWidth;
    int h = m_ImageHeight;
    int stride = m_image->GetWidth();

    int bestVal = INT_MAX;
    int bestCol = -1;

    int rowStart = (h - 1) * stride;
    for (int c = 0; c < w; c++)
    {
        if (m_dp[rowStart + c] < bestVal)
        {
            bestVal = m_dp[rowStart + c];
            bestCol = c;
        }
    }

    assert(bestCol != -1);

    m_seam[h - 1] = bestCol;
    m_minSeam = m_maxSeam = bestCol;

    rowStart = (h - 2) * stride;

    for (int row = h - 2; row >= 0; row--, rowStart -= stride)
    {
        int left = (bestCol <= 0 ? 0 : bestCol - 1);
        int right = (bestCol >= w - 1 ? w - 1 : bestCol + 1);

        if (m_dp[rowStart + left] == m_dp[rowStart + right])
        {
            if (m_dp[rowStart + bestCol] > m_dp[rowStart + left])
            {
                int l = (left <= 0 ? 0 : left - 1);
                int r = (right >= w - 1 ? w - 1 : right + 1);
                bestCol = (m_dp[rowStart + l] == m_dp[rowStart + r]) ?
                                                    left :
                                                    (m_dp[rowStart + l] < m_dp[rowStart + r] ? left : right);
            }
        }
        else if (m_dp[rowStart + left] < m_dp[rowStart + bestCol])
        {
            bestCol = m_dp[rowStart + left] < m_dp[rowStart + right] ? left : right;
        }
        else
        {
            bestCol = (m_dp[rowStart + bestCol] < m_dp[rowStart + right]) ? bestCol : right;
        }

        m_seam[row] = bestCol;


        if (m_minSeam > bestCol)
        {
            m_minSeam = bestCol;
        }
        if (m_maxSeam < bestCol)
        {
            m_maxSeam = bestCol;
        }
    }
}

void SeamCarving::RemoveVerticalSeam()
{
    int w = m_ImageWidth;
    int h = m_ImageHeight;
    int stride = m_image->GetWidth();

    int imageWidth = m_image->GetWidth();

    SimpleImage<BYTE>::Pixel* pixels = m_image->GetData();

    for (int row = 0, rowIndex = 0; row < h; row++, rowIndex += stride)
    {
        int s = m_seam[row];
        memcpy(&m_energy[0] + rowIndex + s, &m_energy[0] + rowIndex + (s+1), sizeof(int) * (w-s-1));
        memcpy(&m_luminosity[0] + rowIndex + s, &m_luminosity[0] + rowIndex + s + 1, sizeof(int) * (w-s-1));
        memcpy(&m_dp[0] + rowIndex + s, &m_dp[0] + rowIndex + s + 1, sizeof(int) * (w-s-1));  
        memcpy(pixels + rowIndex + s, pixels + rowIndex + s + 1, sizeof(int) * (w-s-1));  
    }

    m_ImageWidth--;

    UpdateVerticalEnergy();
}

void SeamCarving::UpdateVerticalEnergy()
{
    int w = m_ImageWidth + 1;
    int h = m_ImageHeight;

    int oldSeamCol = m_seam[0];
    int stride = m_image->GetWidth();

    if (m_seam[0] > 0)
    {
        ComputeLuminanceDifference(0, m_seam[0] - 1, stride);
    }
    if (m_seam[0] < w - 1)
    {
        ComputeLuminanceDifference(0, m_seam[0], stride);
    }

    for (int r = 0; r < h; r++)
    {
        int c = m_seam[r];

        if (r < h - 1)
        {
            int nextSeam = m_seam[r + 1];

            if (nextSeam == c)
            {
                //Pixel to SE
                if (c > 0)
                {
                    ComputeLuminanceDifference(r+1, c-1, stride);
                }
                //Pixel to SW
                if (c < w - 2)
                {
                    ComputeLuminanceDifference(r+1, c, stride);
                }
            }
            else if (nextSeam == c - 1)
            {
                //Pixel to S
                ComputeLuminanceDifference(r+1, c-1, stride);

                //Pixel to SE
                if (c < w - 1)
                {
                    ComputeLuminanceDifference(r+1, c, stride);
                }
                //Pixel to SW
                if (c >= 2)
                {
                    ComputeLuminanceDifference(r+1, c-2, stride);

                    if (oldSeamCol != c - 1)
                    {                        
                        ComputeLuminanceDifference(r, c-2, stride);
                    }
                }
            }
            else
            {
                //Pixel to S
                ComputeLuminanceDifference(r+1, c, stride);

                //Pixel to SW
                if (c >= 1)
                {
                    ComputeLuminanceDifference(r+1, c-1, stride);
                }

                //Pixel to SE
                if (c < w - 2)
                {
                    
                    ComputeLuminanceDifference(r+1, c+1, stride);

                    if (oldSeamCol != c + 1)
                    {                        
                        ComputeLuminanceDifference(r, c+1, stride);
                    }
                }
            }
        }

        oldSeamCol = c;
    }
}

void SeamCarving::ComputeHorizontalDP()
{
    int w = m_ImageWidth;
    int h = m_ImageHeight;
    int stride = m_image->GetWidth();

    if (m_energyDirty)
    {
        ComputeEnergy();
    }

    // Copy first col 
    if (m_lastDpRun != HORIZONTAL)
    {
        for (int r = 0; r < h; r++)
        {
            m_dp[r * stride] = m_energy[r * stride];
        }
    }

    if (m_lastDpRun != HORIZONTAL)
    {
        // Compute rest of dp 
        for (int c = 1; c < w; c++)
        {
            // Special case first and last element 
            m_dp[c] = m_energy[c] + min(m_dp[c - 1], m_dp[stride + c - 1]);
            m_dp[(h - 1) * stride + c] = m_energy[(h - 1) * stride + c] + min(m_dp[(h - 2) * stride + (c - 1)], m_dp[(h - 1) * stride + (c - 1)]);

            // Compute rest of the col 
            for (int r = 1, rowAbove = 0, rowCurrent = stride, rowBelow = stride + stride;
                     r < h - 1;
                     r++, rowAbove = rowCurrent, rowCurrent = rowBelow, rowBelow += stride)
            {
                int min = ::min(m_dp[rowAbove + c - 1], m_dp[rowCurrent + c - 1]);
                min = ::min(min, m_dp[rowBelow + c - 1]);
                m_dp[rowCurrent + c] = m_energy[rowCurrent + c] + min;
            }
        }
    }

    else
    {
        int rMin = max(1, m_seam[0] - 2);
        int rMax = min(m_seam[0] + 3, h - 1);

        int rMinIndex = rMin * stride;
        int rMaxIndex = (rMax - 1) * stride;

        // Compute rest of dp 
        for (int c = 1; c < w; c++)
        {
            // Special case first and last element 
            m_dp[c] = m_energy[c] + min(m_dp[c - 1], m_dp[stride + c - 1]);
            m_dp[(h - 1) * stride + c] = m_energy[(h - 1) * stride + c] + min(m_dp[(h - 2) * stride + (c - 1)], m_dp[(h - 1) * stride + (c - 1)]);

            int rMinOld = m_dp[c + rMinIndex];
            int rMaxOld = m_dp[c + rMaxIndex];            

            bool isTopMin = false;
            int rNextTop = 0;
            {
                int rMinTop = rMin > 1 ? m_dp[c + rMinIndex - 2*stride] : INT_MAX;
                int rMinCurr = m_dp[c + rMinIndex - stride];

                if (rMinOld < rMinTop && rMinOld < rMinCurr)
                {
                    isTopMin = true;
                }
                else
                {
                    rNextTop = min(rMinTop, rMinCurr);
                }
            }

            bool isBotMin = false;
            int rNextBot = 0;
            {
                int rMaxCurr = m_dp[c + rMaxIndex + stride];                
                int rMaxBot = rMax < (h - 1) ? m_dp[c + rMaxIndex + stride*2] : INT_MAX;

                if (rMaxOld < rMaxBot && rMaxOld < rMaxCurr)
                {
                    isBotMin = true;
                }
                else
                {
                    rNextBot = min(rMaxBot, rMaxCurr);
                }
            }  

            // Compute rest of the col             
            for (int r = rMin, rowAbove = rMinIndex - stride + c, rowCurrent = rMinIndex + c, rowBelow = rMinIndex + stride + c;
                     r < rMax;
                     r++, rowAbove = rowCurrent, rowCurrent = rowBelow, rowBelow += stride)
            {
                int min = ::min(m_dp[rowAbove - 1], m_dp[rowCurrent - 1]);
                min = ::min(min, m_dp[rowBelow - 1]);
                m_dp[rowCurrent] = m_energy[rowCurrent] + min;
            }
            
            
            if (rMin > 1 && rMinOld != m_dp[rMinIndex + c] &&
                (isTopMin || m_dp[rMinIndex + c] < rNextTop))
            {
                rMin--;
                rMinIndex -= stride;
            }
            if (rMax < h - 1 && rMaxOld != m_dp[rMaxIndex + c] &&
                (isBotMin || m_dp[rMaxIndex + c] < rNextBot))
            {
                rMax++;
                rMaxIndex += stride;
            }
        }
    }

    m_lastDpRun = HORIZONTAL;

}

void SeamCarving::ComputeHorizontalSeam()
{
    int w = m_ImageWidth;
    int h = m_ImageHeight;
    int stride = m_image->GetWidth();

    int bestVal = INT_MAX;
    int bestRow = -1;

    for (int r = 0, rowIndex = w - 1; r < h; r++, rowIndex += stride)
    {
        if (m_dp[rowIndex] < bestVal)
        {
            bestVal = m_dp[rowIndex];
            bestRow = r;
        }
    }

    assert(bestRow != -1);

    m_seam[w - 1] = bestRow;
    m_maxSeam = m_minSeam = bestRow;

    for (int col = w - 2; col >= 0; col--)
    {
        int top = (bestRow <= 0 ? 0 : bestRow - 1);
        int bot = (bestRow >= h - 1 ? h - 1 : bestRow + 1);

        int topIndex = top * stride + col;
        int botIndex = bot * stride + col;
        int bestIndex = bestRow * stride + col;

        if (m_dp[topIndex] == m_dp[botIndex])
        {
            if (m_dp[bestIndex] > m_dp[topIndex])
            {
                int t = (top <= 0 ? 0 : top - 1) * stride + col;
                int b = (bot >= h - 1 ? h - 1 : bot + 1) * stride + col;
                bestRow = (m_dp[t] == m_dp[b]) ?
                                                    top :
                                                    (m_dp[t] < m_dp[b] ? top : bot);
            }
        }
        else if (m_dp[topIndex] < m_dp[bestIndex])
        {
            bestRow = m_dp[topIndex] < m_dp[botIndex] ? top : bot;
        }
        else
        {
            bestRow = m_dp[bestIndex] < m_dp[botIndex] ? bestRow : bot;
        }

        m_seam[col] = bestRow;
        if (m_minSeam > bestRow)
        {
            m_minSeam = bestRow;
        }
        if (m_maxSeam < bestRow)
        {
            m_maxSeam = bestRow;
        }

    }
}

void SeamCarving::RemoveHorizontalSeam()
{
    int w = m_ImageWidth;
    int h = m_ImageHeight;
    int stride = m_image->GetWidth();
    
    SimpleImage<BYTE>::Pixel* pixels = m_image->GetData();
        
    for (int r = m_minSeam, rIndex = m_minSeam * stride; r < m_maxSeam; r++, rIndex += stride)
    {
        for (int c = 0; c < w; c++)
        {
            if (r < m_seam[c])
            {
                continue;
            }

            pixels[rIndex + c] = pixels[rIndex + stride + c];
            m_luminosity[rIndex + c] = m_luminosity[rIndex + stride + c];
            m_energy[rIndex + c] = m_energy[rIndex + stride + c];
            m_dp[rIndex + c] = m_dp[rIndex + stride + c];
        }
    }
    
    for (int r = m_maxSeam, rIndex = m_maxSeam * stride; r < h - 1; r++, rIndex += stride)
    {
        memcpy(pixels + rIndex, pixels + rIndex + stride, w * sizeof(SimpleImage<BYTE>::Pixel));
        memcpy(&m_luminosity[0] + rIndex, &m_luminosity[0] + rIndex + stride, sizeof(int) * w);
        memcpy(&m_energy[0] + rIndex, &m_energy[0] + rIndex + stride, sizeof(int) * w);
        memcpy(&m_dp[0] + rIndex, &m_dp[0] + rIndex + stride, sizeof(int) * w);
    }


    m_ImageHeight--;
    UpdateHorizontalEnergy();
}

void SeamCarving::UpdateHorizontalEnergy()
{
    int w = m_ImageWidth;
    int h = m_ImageHeight + 1;
    int stride = m_image->GetWidth();
    
    int oldSeamRow = m_seam[0];

    if (m_seam[0] > 0)
    {
        ComputeLuminanceDifference(m_seam[0] - 1, 0, stride);
    }
    if (m_seam[0] < h - 1)
    {
        ComputeLuminanceDifference(m_seam[0], 0, stride);
    }


    for (int c = 0; c < w; c++)
    {
        int r = m_seam[c];
            
        if (c < w - 1)
        {
            int nextSeam = m_seam[c + 1];

            if (nextSeam == r)
            {
                //Pixel to NW
                if (r > 0)
                {
                    ComputeLuminanceDifference(r-1, c+1, stride);
                }
                //Pixel to SW
                if (r < h - 2)
                {
                    ComputeLuminanceDifference(r, c+1, stride);
                }
            }
            else if (nextSeam == r - 1)
            {
                //Pixel to W
                ComputeLuminanceDifference(r-1, c+1, stride);

                //Pixel to SW
                if (r < h - 1)
                {
                    ComputeLuminanceDifference(r, c+1, stride);
                }

                //Pixel to NW
                if (r >= 2)
                {
                    ComputeLuminanceDifference(r-2, c+1, stride);

                    if (oldSeamRow != r - 1)
                    {
                        ComputeLuminanceDifference(r-2, c, stride);
                    }
                }
            }
            else
            {
                //Pixel to W
                ComputeLuminanceDifference(r, c+1, stride);

                //Pixel to NW
                if (r >= 1)
                {
                    ComputeLuminanceDifference(r-1, c+1, stride);
                }

                //Pixel to SW
                if (r < h - 2)
                {
                    ComputeLuminanceDifference(r+1, c+1, stride);

                    if (oldSeamRow != r + 1)
                    {
                        ComputeLuminanceDifference(r+1, c, stride);
                    }
                }
            }
        }
        oldSeamRow = r;
    }
}
