#pragma once
#include "ApolloCommon.h"
#include "Image.h"

namespace Apollo
{
class PatchMatch
{
    public:
        enum PatchDistance
        {
            RGB_L2,
            Luminance_L2,
        };

    public:
        // Computes patch match on the region of the dst image using the src image.
        // Only the region of the dst image will be modified.
        // To patch match the entire image, pass null as the region
        // To patch match a portion, copy the src image to the dst and specify the region of interest
        // The dst image should be allocated to the proper size.  It does not have to match the src
        // image's dimensions.
        static void ComputePatchMatch(const Image* src, Image* dst, const Rect<int>* region, PatchDistance distanceMetric, int nIterations=5);

    private:
        struct PatchMatchData
        {
            const Image* src;
            Image* dst;
            Rect<int> region;
            PatchDistance metric;
            std::vector<int> mapping;
            std::vector<float> errors;
            FilterKernel* kernel;
        };

        struct ImagePatch
        {
            const Image* image;
            UINT row;
            UINT col;
            UINT size;

            // Paper recommends 7x7 patches
            ImagePatch(const Image* img, UINT r, UINT c, UINT s=s_PatchSize)
            {
                image = img;
                row = r;
                col = c;
                size = s;
                assert(size % 2 == 1);
            }
        };
    
    private:
        static void ComposeImage(PatchMatchData& data, Image* dst);
        static void UpdateMapping(PatchMatchData& data, int index, int offset, float error);
        static void Propagate(PatchMatchData& data, const ImagePatch& dstPatch, int regionIndex, bool reverse);
        static void RandomSearch(PatchMatchData& data, const ImagePatch& dstPatch, int regionIndex);
        static int RegionToImageIndex(const PatchMatchData& data, int index);
        static float ComputePatchDistance(const ImagePatch& src, const ImagePatch& dst, const FilterKernel* kernel, Rect<int>* region, PatchDistance metric, float max);

        // Debugging
        static float ComputeAverageError(const PatchMatchData& data);

        static const UINT s_PatchSize = 7;
    };
}
