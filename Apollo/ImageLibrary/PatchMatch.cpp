#include "PatchMatch.h"

namespace Apollo
{
void PatchMatch::ComputePatchMatch(const Image* src, Image* dst, const Rect<int>* region, PatchDistance metric, int nIterations)
{
    int regionSize = region->w * region->h;
    int srcW = src->GetWidth();
    int srcH = src->GetHeight();

    PatchMatchData data;
    data.src = src;
    data.dst = dst;
    data.region = *region;
    data.metric = metric;
    data.mapping.resize(regionSize);
    data.errors.resize(regionSize);

    // Create a gaussian kernel for waiting patches.  No idea if this helps
    FilterKernel gaussian;
    FilterKernel::CreateGaussianFilter2D(&gaussian, s_PatchSize, 1.5);
    data.kernel = &gaussian;

    // Initialize mapping to be all random
    for (int i = 0; i < regionSize; i++)
    {
        int randRow = rand() % srcH;
        int randCol = rand() % srcW;

        ImagePatch srcPatch(src, randRow, randCol);
        ImagePatch dstPatch(dst, i / region->w + region->y, i % region->w + region->x);

        float f = ComputePatchDistance(srcPatch, dstPatch, data.kernel, &data.region, data.metric, INFINITY);
        UpdateMapping(data, i, randRow * srcW + randCol, f);
    }

    Image* temp = new Image(*dst);
    ComposeImage(data, temp);
    ImageIO::Save("E:\\Data\\Init.png", temp);
    printf("Done random seed: %f\n", ComputeAverageError(data));

    for (int n = 0; n < nIterations; n++)
    {
        // As suggested by the paper, every other iteration should go backwards
        int start = 0;
        int end = regionSize;
        int delta = 1;
        if (n % 2 == 1)
        {
            start = regionSize - 1;
            end = -1;
            delta = -1;
        }

        // Patch match uses a local search to find good patches.  It has two parts:
        //   1. Propagate - this is the greedy part where we see if we can find good matches from our neighbors
        //   2. RandomSearch - this gets out of local minima by looking randomly in other places

        /*
        for (int i = start; i != end; i += delta)
        {
            // f0 = D(f(x, y))
            int dstIndex = RegionToImageIndex(data, i);
            int dstRow = dstIndex / dst->GetWidth();
            int dstCol = dstIndex % dst->GetWidth();

            ImagePatch dstPatch(dst, dstRow, dstCol);
            
            Propagate(data, dstPatch, i, (n % 2) == 1);
            RandomSearch(data, dstPatch, i);
        }
        */
        // dlev's suggestion to use 2 propagates per random search.
        for (int i = 0; i != regionSize; i++)
        {
            // f0 = D(f(x, y))
            int dstIndex = RegionToImageIndex(data, i);
            int dstRow = dstIndex / dst->GetWidth();
            int dstCol = dstIndex % dst->GetWidth();
            ImagePatch dstPatch(dst, dstRow, dstCol);
            Propagate(data, dstPatch, i, false);
        }
        for (int i = regionSize; i != -1; i--)
        {
            // f0 = D(f(x, y))
            int dstIndex = RegionToImageIndex(data, i);
            int dstRow = dstIndex / dst->GetWidth();
            int dstCol = dstIndex % dst->GetWidth();
            ImagePatch dstPatch(dst, dstRow, dstCol);
            Propagate(data, dstPatch, i, true);
        }
        for (int i = 0; i != regionSize; i++)
        {
            // f0 = D(f(x, y))
            int dstIndex = RegionToImageIndex(data, i);
            int dstRow = dstIndex / dst->GetWidth();
            int dstCol = dstIndex % dst->GetWidth();
            ImagePatch dstPatch(dst, dstRow, dstCol);
            RandomSearch(data, dstPatch, i);
        }

#pragma warning (disable : 4996)
        ComposeImage(data, temp);
        char buf[1024];
        sprintf(buf, "E:\\Data\\Iter%d.png", n + 1);
        ImageIO::Save(buf, temp);
        printf("Finished iteration: %d (%f)\n", n + 1, ComputeAverageError(data));
#pragma warning (default : 4996)
    }

    delete temp;

    // Making the final mapping
    ComposeImage(data, data.dst);
}

void PatchMatch::ComposeImage(PatchMatchData& data, Image* dst)
{
    int index = 0;
    for (int row = 0; row < data.region.h; row++)
    {
        for (int col = 0; col < data.region.w; col++)
        {
            int offset = data.mapping[index++];

            int dstIndex = (row + data.region.y) * dst->GetWidth() +(col + data.region.x);
            int srcIndex = dstIndex + offset;
            dst->GetData()[dstIndex] = data.src->GetData()[srcIndex];
        }
    }
}

float PatchMatch::ComputeAverageError(const PatchMatchData& data)
{
    float error = 0;
    for (UINT i = 0; i < data.errors.size(); i++)
    {
        error += data.errors[i];
    }
    return error / data.errors.size();
}

// Translates an index from the region of interest to the index into the dst image
int PatchMatch::RegionToImageIndex(const PatchMatchData& data, int index)
{
    int regionRow = index / data.region.w;
    int regionCol = index % data.region.w;

    int dstRow = regionRow + data.region.y;
    int dstCol = regionCol + data.region.x;

    return dstRow * data.dst->GetWidth() + dstCol;
}

// regionIndex is the index into the region
// srcIndex is the index into to srcImage (NOT offset)
void PatchMatch::UpdateMapping(PatchMatchData& data, int regionIndex, int srcIndex, float error)
{
    assert (regionIndex >= 0 && regionIndex < (int)data.mapping.size());
    assert (srcIndex >= 0 &&srcIndex < (int)data.src->GetSize());
    int dstIndex = RegionToImageIndex(data, regionIndex);

    // Compute the offset
    data.mapping[regionIndex] = srcIndex - dstIndex;
    data.errors[regionIndex] = error;
}

// Randomly search around the current offset in smaller and smaller windows
// f0 is the distance of the current patch mapping
void PatchMatch::RandomSearch(PatchMatchData& data, const ImagePatch& dstPatch, int regionIndex)
{
    int srcW = data.src->GetWidth();
    int srcH = data.src->GetHeight();

    int searchWindow = MAX(srcW, srcH);

    // (srcRow, srcCol) is index into the src image
    int dstIndex = RegionToImageIndex(data, regionIndex);
    int srcIndex = data.mapping[regionIndex] + dstIndex;
    int srcRow = srcIndex / srcW;
    int srcCol = srcIndex % srcH;

    // As suggested by the paper, start with a random window that's the entire size of the src image,
    // decrease the window by half each iteration.
    while (searchWindow > 1)
    {
        int minRow = MAX(0, srcRow - searchWindow);
        int minCol = MAX(0, srcCol - searchWindow);
        int maxRow = MIN(srcH, srcRow + searchWindow);
        int maxCol = MIN(srcW, srcCol + searchWindow);

        int randomRow = rand() % (maxRow - minRow) + minRow;
        int randomCol = rand() % (maxCol - minCol) + minCol;

        ImagePatch srcPatch(data.src, randomRow, randomCol);
        float f = ComputePatchDistance(srcPatch, dstPatch, data.kernel, &data.region, data.metric, data.errors[regionIndex]);

        if (f < data.errors[regionIndex])
        {
            UpdateMapping(data, regionIndex, randomRow * srcW + randomCol, f);
        }
    
        searchWindow /= 2;
    }
}

// Attempt to propagate my offset from my neighbor
// f0 is the distance of the current patch mapping
void PatchMatch::Propagate(PatchMatchData& data, const ImagePatch& dstPatch, int regionIndex, bool reverse)
{
    // At each iteration we want to propogate (local search) and then random search and pickt he best result
    // As advised in the paper, for even iterations, we'll propagate Top&Left, for odd Down&Right
    
    int srcW = data.src->GetWidth();
    int dstIndex = RegionToImageIndex(data, regionIndex);

    int regionRow0 = regionIndex / data.region.w;
    int regionCol0 = regionIndex % data.region.w;
    int regionRow1, regionCol1;

    int srcIndex1 = -1;
    int srcIndex2 = -1;

    if (reverse)
    {
        regionRow1 = regionRow0 + 1;
        regionCol1 = regionCol0 + 1;
    }
    else
    {
        regionRow1 = regionRow0 - 1;
        regionCol1 = regionCol0 - 1;
    }

    int offset1, offset2;

    if (regionCol1 >= 0 && regionCol1 < data.region.w)
    {
        offset1 = data.mapping[regionRow0 * data.region.w + regionCol1];
        srcIndex1 = offset1 + dstIndex;
    }
    if (regionRow1 >= 0 && regionRow1 < data.region.h)
    {
        offset2 = data.mapping[regionRow1 * data.region.w + regionCol0];
        srcIndex2 = offset2 + dstIndex;
    }

    // D(f(x - 1, y))
    if (srcIndex1 >= 0 && srcIndex1 < (int)data.src->GetSize())
    {
        ImagePatch srcPatch(data.src, srcIndex1 / srcW, srcIndex1 % srcW);
        float f = ComputePatchDistance(srcPatch, dstPatch, data.kernel, &data.region, data.metric, data.errors[regionIndex]);
    
        if (f < data.errors[regionIndex])
        {
            UpdateMapping(data, regionIndex, srcIndex1, f);
        }
    }
    
    // D(f(x, y - 1))
    if (srcIndex2 >= 0 && srcIndex2 < (int)data.src->GetSize())
    {
        ImagePatch srcPatch(data.src, srcIndex2 / srcW, srcIndex2 % srcW);
        float f = ComputePatchDistance(srcPatch, dstPatch, data.kernel, &data.region, data.metric, data.errors[regionIndex]);

        if (f < data.errors[regionIndex])
        {
            UpdateMapping(data, regionIndex, srcIndex2, f);
        }
    }
}

float PatchMatch::ComputePatchDistance(const ImagePatch& src, const ImagePatch& dst, const FilterKernel* kernel, Rect<int>* region, PatchDistance type, float max)
{
    assert(src.size == dst.size);
    assert(src.size % 2 == 1);
    assert(src.image->GetWidth() > src.size && src.image->GetHeight() > src.size);
    assert(dst.image->GetWidth() > dst.size && dst.image->GetHeight() > dst.size);

    // This is a really inefficient way to do this.  The code below is faster but wrong (can be fixed).
    // I think this approach does handle edges correctly.
    float totalError = 0;
    int n = 0;
    int size = src.size / 2;
    int kernelIndex = 0;

    for (int row = -size; row <= size; row++)
    {
        for (int col = -size; col <= size; col++, kernelIndex++)
        {
            int patch1Row = src.row + row;
            int patch1Col = src.col + col;
            int patch2Row = dst.row + row;
            int patch2Col = dst.col + col;

            if (patch1Row < 0 || patch1Row >= (int)src.image->GetHeight() ||
                patch1Col < 0 || patch1Col >= (int)src.image->GetWidth() ||
                patch2Row < 0 || patch2Row >= (int)dst.image->GetHeight() ||
                patch2Col < 0 || patch2Col >= (int)dst.image->GetWidth())
            {
                continue;
            }

            // This patch contains pixels in the masked out (to be scene complete region), return large distance, pixels cant be used
            if (region && region->Contains(patch1Row, patch1Col))
            {
                return max;
            }

            const Color4f& pixel1 = src.image->GetPixel(patch1Row, patch1Col);
            const Color4f& pixel2 = dst.image->GetPixel(patch2Row, patch2Col);

            float dr = pixel1.r - pixel2.r;
            float dg = pixel1.g - pixel2.g;
            float db = pixel1.b - pixel2.b;

            totalError += (dr*dr + dg*dg + db*db) * kernel->kernel[kernelIndex];
            n++;
        }

        if (totalError / n > max)
        {
            break;
        }
    }

    return totalError / n;
    /*
    // Crop the patch to be within the bounds for both images
    UINT p1Row, p1Col, p2Row, p2Col;
    UINT iW1, iW2, iH1, iH2;
    UINT size;

    size = patch1.size / 2;
    iW1 = patch1.image->GetWidth();
    iW2 = patch2.image->GetWidth();
    iH1 = patch1.image->GetHeight();
    iH2 = patch2.image->GetHeight();
    
    int p1dWL = patch1.col;
    int p1dWR = iW1 - patch1.col - 1;
    int p1dHT = patch1.row;
    int p1dHB = iH1 - patch1.row - 1;
    
    int p2dWL = patch2.col;
    int p2dWR = iW2 - patch2.col - 1;
    int p2dHT = patch2.row;
    int p2dHB = iH2 - patch2.row - 1;

    assert (p1dWL >= 0 && p1dWR >= 0 && p1dHT >= 0 && p1dHB >= 0);
    assert (p2dWL >= 0 && p2dWR >= 0 && p2dHT >= 0 && p2dHB >= 0);

    // Take the smallest patch possible.  Should I try taking no square patches on the borders?
    size = MIN(size, (UINT)p1dWL);
    size = MIN(size, (UINT)p1dWR);
    size = MIN(size, (UINT)p1dHT);
    size = MIN(size, (UINT)p1dHB);
    size = MIN(size, (UINT)p2dWL);
    size = MIN(size, (UINT)p2dWR);
    size = MIN(size, (UINT)p2dHT);
    size = MIN(size, (UINT)p2dHB);

    UINT size2 = size*2 + 1;

    p1Row = patch1.row - size;
    p1Col = patch1.col - size;
    p2Row = patch2.row - size;
    p2Col = patch2.col - size;

    assert (p1Row >= 0 && p1Col >= 0 && p2Row >= 0 && p2Col >= 0);
    assert (patch1.row + size < iH1 && patch1.col + size < iW1);
    assert (patch2.row + size < iH2 && patch2.col + size < iW2);

    const Color4f* data1 = patch1.image->GetData();
    const Color4f* data2 = patch2.image->GetData();
    
    float totalError = 0;

    if (type == RGB_L2)
    {
        for (UINT row = 0; row < size2; row++)
        {
            const Color4f* data1Ptr = data1 + (p1Row + row)*iW1 + p1Col;
            const Color4f* data2Ptr = data2 + (p2Row + row)*iW2 + p2Col;

            for (UINT col = 0; col < size2; col++, data1Ptr++, data2Ptr++)
            {
                float dr = data1Ptr->r - data2Ptr->r;
                float dg = data1Ptr->g - data2Ptr->g;
                float db = data1Ptr->b - data2Ptr->b;

                totalError += dr*dr + dg*dg + db*db;
            }
        }
    }
    else if (type == Luminance_L2)
    {
        for (UINT row = 0; row < size2; row++)
        {
            const Color4f* data1Ptr = data1 + (p1Row + row)*iW1 + p1Col;
            const Color4f* data2Ptr = data2 + (p2Row + row)*iW2 + p2Col;

            for (UINT col = 0; col < size2; col++, data1Ptr++, data2Ptr++)
            {
                float l1 = data1Ptr->ToLuminance();
                float l2 = data2Ptr->ToLuminance();
                totalError += (l1 - l2) * (l1 - l2);
            }
        }
    }
    else
    {
        assert(false);
    }

    // Return average error (squared) to account for cropped patch regions.  I think this
    // is the right thing to do for borders
    return totalError / (size2*size2);
    */
}

}