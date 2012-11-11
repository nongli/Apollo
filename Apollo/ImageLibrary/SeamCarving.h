#pragma once
#include <vector>

#include "Image.h"

namespace Apollo {

class SeamCarving {
public:
    // SeamCacheNone - no seams are ever cached
    // SeamCachePrecompute - On init, all the seams are precomputed.
    // SeamCacheDynamic - Nothing is computed on Init but seams will be cached when the engine hits that size.
    enum SeamCacheType {
        SeamCacheNone,
        SeamCachePrecompute,
        SeamCacheDynamic
    };

public:
    /// <summary>
    /// Creates the seam carving for this image.  The image passed is assumed
    /// to not be changed later.  Changing the image will probably crash
    /// seam carving.  
    /// </summary>
    /// <param name="image">The image to seam carve.</param>
    /// <param name="result">The resulting image buffer.</param>
    /// <param name="precompute">How should the engine deal with seam caching</param>
    SeamCarving(SimpleImage<BYTE>* image, SimpleImage<BYTE>* result, SeamCacheType seamCacheType);

    /// <summary>
    /// Seam carves the image to width x height. It is not possible (right now) to seam carve to a
    /// image bigger than the original.  
    /// </summary>
    /// <param name="width">New Width</param>
    /// <param name="height">New Height</param>
    /// <returns> A bitmap containing hte image at the requested size.</returns>
    SimpleImage<BYTE>* SeamCarve(int width, int height);

    /// <summary>
    /// Returns the width of the current seam carved image.
    /// </summary>
    /// <returns></returns>
    int GetWidth() const {
        return m_ImageWidth;
    }

    /// <summary>
    /// Returns the height of the current seam carved image.
    /// </summary>
    /// <returns></returns>
    int GetHeight() const {
        return m_ImageHeight;
    }

    ~SeamCarving();

private:
    // Members
    SimpleImage<BYTE>* m_image;
    SimpleImage<BYTE>* m_originalImage;

    int m_ImageWidth;
    int m_ImageHeight;

    std::vector<int> m_luminosity;
    std::vector<int> m_energy;
    std::vector<int> m_dp;
    std::vector<int> m_seam;

    int m_minSeam;
    int m_maxSeam;

    bool m_energyDirty;

    SeamCacheType m_seamCacheType;
    class SeamCarveCacheHorizontal* m_horizontalCache;
    class SeamCarveCacheVertical* m_verticalCache;

    enum LastDpRun {
        NONE,
        VERTICAL,
        HORIZONTAL
    };
    LastDpRun m_lastDpRun;

    // Helpers
private:
    void ComputeLuminanceDifference(int r, int c, int w);
    void Reset();

    void ComputeEnergy();
    void PrecomputeHorizontalSeams();
    void PrecomputeVerticalSeams();

    void ComputeVerticalDP();
    void ComputeVerticalSeam();
    void RemoveVerticalSeam();
    void UpdateVerticalEnergy();

    void ComputeHorizontalDP();
    void ComputeHorizontalSeam();
    void RemoveHorizontalSeam();
    void UpdateHorizontalEnergy();
};

}