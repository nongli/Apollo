#include "ApolloCommon.h"
#include "ImageAnalogy.h"
#include "ImageAnalogyImpl.h"

namespace Apollo
{
    ImageAnalogy::ImageAnalogy(void) 
    {
        m_levels					=	5;
        m_luminanceRemapping		=	false;
        m_saveIntermediate			=	false;
        m_featureType				=	FEATURE_LUMINANCE;
        m_postProcessType			=	POSTPROCESS_B_COLORS;
        m_error						=	.05f;
        m_coherence					=	1.0f;
    }

    ImageAnalogy::~ImageAnalogy(void) 
    {
    }

    Image* ImageAnalogy::createAnalogyPatchMatch(const Image* A, const Image* Aprime, const Image* B) 
    {
        assert(A->GetWidth() == Aprime->GetWidth());
        assert(A->GetHeight() == Aprime->GetHeight());

        ILogger::Logger()->Status("Beginning image analogy computation using PatchMatch.");

        Image* aTemp = new Image(*A);
        Image* aPrimeTemp = new Image(*Aprime);

        if (m_luminanceRemapping)
        {
            remapLuminance(aTemp, B);
            remapLuminance(aPrimeTemp, B);
        }

        ILogger::Logger()->Status("Computing gaussian pyramid for %d levels.", m_levels);

        // Compute gaussian pyramid for 3 input images
        GaussianPyramid* APyramid = GaussianPyramid::Create(aTemp);
        GaussianPyramid* APrimePyramid = GaussianPyramid::Create(aPrimeTemp);
        GaussianPyramid* BPyramid = GaussianPyramid::Create(B);
        m_levels = MIN(m_levels, APyramid->GetNumLevels());
        m_levels = MIN(m_levels, APrimePyramid->GetNumLevels());
        m_levels = MIN(m_levels, BPyramid->GetNumLevels());

        ILogger::Logger()->Status("Generating Result Image.");
        TaskData task("Generating Result Image", 1, 0, true);

        /* Allocate space for the resulting images */
        Image** BImages = new Image*[m_levels];
        for (UINT i = 0; i < m_levels; i++) 
        {
            BImages[i] = new Image(BPyramid->GetImageAt(i)->GetWidth(), BPyramid->GetImageAt(i)->GetHeight());
        }

        // Generate the resulting image starting at the lowest resolution
        ImageAnalogyDataPatchMatch data;
        data.ALow = nullptr;
        data.BLow = nullptr;
        data.APLow = nullptr;
        data.BPLow = nullptr;

        // This is a modifed image analogy approach using patch match to generate the Bprime images
        // Differences between this approach and standard image analogies
        //    1. Instead of ANNsearch & Cohenernce Search, use PatchMatch (RandomSearch & Propagate)
        //    2. Instead of using partial neighborhoods, use the full neighborhood
        for (int level = m_levels - 1; level >= 0; level--) 
        {
            data.AHigh = APyramid->GetImageAt(level);
            data.APHigh = APrimePyramid->GetImageAt(level);
            data.BHigh = BPyramid->GetImageAt(level);
            data.BPHigh = BImages[level];
            data.level = level;
            data.mapping.resize(data.BHigh->GetSize(), -1);
            
            /* Run the algorithm on this level */
            constructTargetImagePatchMatch(&data, &task);

            /* Post process to remap the colors correctly */
            postProcess(APyramid->GetImageAt(level), APrimePyramid->GetImageAt(level), BPyramid->GetImageAt(level), BImages[level]);

            data.ALow = data.AHigh;
            data.BLow = data.BHigh;
            data.APLow = data.APHigh;
            data.BPLow = data.BPHigh;
        }

        // Cleanup
        for (UINT i = 1; i < m_levels; i++) 
        {
            delete BImages[i];
        }
        delete aTemp;
        delete aPrimeTemp;
        return nullptr;
    }

    Image* ImageAnalogy::createAnalogy(const Image* A, const Image* Aprime, const Image* B) 
    {
        assert(A->GetWidth() == Aprime->GetWidth());
        assert(A->GetHeight() == Aprime->GetHeight());

        ILogger::Logger()->Status("Beginning image analogy computation.");

        /* See if we need to do luminance remapping */
        if (m_luminanceRemapping) 
        {
            remapLuminance((Image*)A, B);
            remapLuminance((Image*)Aprime, B);
        }

        ILogger::Logger()->Status("Computing gaussian pyramid for %d levels.", m_levels);
        /* Compute gaussian pyramid for 3 input images */
        GaussianPyramid* APyramid = GaussianPyramid::Create(A);
        GaussianPyramid* APrimePyramid = GaussianPyramid::Create(Aprime);
        GaussianPyramid* BPyramid = GaussianPyramid::Create(B);
        m_levels = MIN(m_levels, APyramid->GetNumLevels());
        m_levels = MIN(m_levels, APrimePyramid->GetNumLevels());
        m_levels = MIN(m_levels, BPyramid->GetNumLevels());

        ILogger::Logger()->Status("Computing image features.");
        /* At each level compute the feature vectors for the src image pairs */
        FeaturePyramid* srcFeatures = FeaturePyramid::CreateFeaturePyramid(APyramid, APrimePyramid, m_featureType);

        ILogger::Logger()->Status("Initializing approximate nearest neighbors search.");
        /* Initialize data structure for ann search */
        ANNSearch** annSearch = new ANNSearch*[m_levels];
        for (UINT i = 0; i < m_levels; i++) 
        {
            UINT numElements;
            double* data = srcFeatures->getFeatures(i, numElements);
            annSearch[i] = new ANNSearch(data, numElements, srcFeatures->getFeatureLength(), m_error);
        }

        TaskData task("Generating Result Image", 1, 0, true);

        ILogger::Logger()->Status("Generating result image.");
        /* Allocate space for the resulting images */
        Image** BImages = new Image*[m_levels];
        for (UINT i = 0; i < m_levels; i++) 
        {
            BImages[i] = new Image(BPyramid->GetImageAt(i)->GetWidth(), BPyramid->GetImageAt(i)->GetHeight());
        }

        /* Generate the resulting image starting at the lowest resolution */
        ImageAnalogyData data;
        data.srcFeatures = srcFeatures;
        data.ALow = nullptr;
        data.BLow = nullptr;
        data.APLow = nullptr;
        data.BPLow = nullptr;

        for (int level = m_levels - 1; level >= 0; level--) 
        {
            data.AHigh = APyramid->GetImageAt(level);
            data.APHigh = APrimePyramid->GetImageAt(level);
            data.BHigh = BPyramid->GetImageAt(level);
            data.BPHigh = BImages[level];
            data.annSearch = annSearch[level];
            data.level = level;
            data.mapping.resize(data.BHigh->GetSize(), -1);
            
            /* Run the algorithm on this level */
            constructTargetImage(&data, &task);

            /* Post process to remap the colors correctly */
            postProcess(APyramid->GetImageAt(level), APrimePyramid->GetImageAt(level), BPyramid->GetImageAt(level), BImages[level]);

            data.ALow = data.AHigh;
            data.BLow = data.BHigh;
            data.APLow = data.APHigh;
            data.BPLow = data.BPHigh;
        }

        Image* Bprime = BImages[0];
        
        /* Clean up allocated memory */
        TaskData cleanup("Cleaning up allocated resources", 5, 1, true);
        TaskData deletePyramid("Deleting Pyramids", 1, 0, true, &cleanup);
        delete APyramid;
        delete APrimePyramid;
        delete BPyramid;
        deletePyramid.Complete();

        
        TaskData deleteFeatures("Deleting srcFeatures", 1, 0, true, &cleanup);
        delete srcFeatures;
        deleteFeatures.Complete();
        
        TaskData deleteANN("Deleting ann", 1, 0, true, &cleanup);
        for (UINT i = 0; i < m_levels; i++) 
        {
            delete annSearch[i];
        }
        delete[] annSearch;
        deleteANN.Complete();

        TaskData deleteImages("Deleting images", 1, 0, true, &cleanup);
        for (UINT i = 1; i < m_levels; i++) 
        {
            delete BImages[i];
        }
        delete[] BImages;
        deleteImages.Complete();

        cleanup.Complete();

        return Bprime;
    }

    /* This will construct one level of the image analogy algorithm.
     * The pixels of resHigh will be filled in scan line order.
     * At every pixel, it will compute the ann and the best coherence match
     * and choose the best between the two.
     */
    void ImageAnalogy::constructTargetImage(ImageAnalogyData* data, TaskData* parent) 
    {
        /* Initialize buffer to place search feature */
        int nDims = data->srcFeatures->getFeatureLength();
        double* searchFeature = new double[nDims];
        UINT bW = data->BHigh->GetWidth();
        UINT bH = data->BHigh->GetHeight();

        float coherenceFactor = 1.0f + powf(2.0f, -(float)data->level) * m_coherence;
      
        TaskData task("Constructing level", 
                        bH, 
                        data->level < 2 ? 10 : 0, 
                        data->level < 2, 
                        parent);

        for (UINT row = 0; row < bH; row++) 
        {
            for (UINT col = 0; col < bW; col++) 
            {
                /* Find the current targetFeature (F(q)) */
                FeaturePyramid::computeFeature(data->BLow, data->BHigh, data->BPLow, data->BPHigh, row, col, searchFeature, m_featureType);

                /* Indices in the src image */
                int pAnn = 0;
                int pCoh = 0;
                double dAnn = 1;
                double dCoh = 1;
                int bestIndex;

                /* Handle edge cases */
                if (data->level == m_levels - 1 || row < 2 || col < 2) 
                {
                    pAnn = data->annSearch->findNN(searchFeature);
                    dAnn = 0;
                }
                else 
                {
                    /* Perform ann search */
                    pAnn = data->annSearch->findANN(searchFeature, dAnn);
                    /* Perform coherence search */
                    pCoh = CoherenceSearch::findBestMatch(data, row, col, searchFeature, dCoh);
                }

                /* Pick coherent pixel */
                if (dCoh <= dAnn*coherenceFactor) 
                {
                    bestIndex = pCoh;
                }
                /* Pick ann pixel */
                else 
                {
                    bestIndex = pAnn;
                }

                /* Place pixel into target image */
                Color4f pixel = data->APHigh->GetPixel(bestIndex);
                data->BPHigh->SetPixel(pixel, row, col);
                data->mapping[row*bW + col] = bestIndex;
            }

            task.SetProgress(row);
        }
        task.Complete();

        delete[] searchFeature;
    }

    // This will construct one level of the image using the PatchMatch algorithm
    void ImageAnalogy::constructTargetImagePatchMatch(ImageAnalogyDataPatchMatch* data, TaskData* parent)
    {
        UINT bW = data->BHigh->GetWidth();
        UINT bH = data->BHigh->GetHeight();
        TaskData task("Constructing level", 
                bH, 
                data->level < 2 ? 10 : 0, 
                data->level < 2, 
                parent);

        int nIterations = 5;

        std::vector<float> f0(m_featureType == FEATURE_LUMINANCE ? 36 : 36 * 3);
        std::vector<float> f1(m_featureType == FEATURE_LUMINANCE ? 36 : 36 * 3);
        std::vector<float> f2(m_featureType == FEATURE_LUMINANCE ? 36 : 36 * 3);

        // Initialize mapping to be all random
        for (UINT i = 0; i < bW * bH; i++)
        {
            int randRow = rand() % data->AHigh->GetHeight();
            int randCol = rand() % data->AHigh->GetWidth();

            int dstRow = i / data->BHigh->GetWidth();
            int dstCol = i % data->BHigh->GetWidth();

            ConstructPatchFeatureVector(data->BHigh, data->BLow, dstRow, dstCol, f0);
            ConstructPatchFeatureVector(data->AHigh, data->ALow, randRow, randCol, f1);
           
            float f = computeFeatureDistance(f0, f1);
            //pdateMapping(data, i, randRow * srcW + randCol, f);
        }

        for (int n = 0; n < nIterations; n++)
        {
            // As suggested by the paper, every other iteration should go backwards
            int start = 0;
            int end = (int)(bW * bH);
            int delta = 1;
            if (n % 2 == 1)
            {
                start = end - 1;
                end = -1;
                delta = -1;
            }

            // Patch match uses a local search to find good patches.  It has two parts:
            //   1. Propagate - this is the greedy part where we see if we can find good matches from our neighbors
            //   2. RandomSearch - this gets out of local minima by looking randomly in other places


            for (int i = start; i != end; i += delta)
            {
                // f0 = D(f(x, y))
                int dstRow = i / data->BHigh->GetWidth();
                int dstCol = i % data->BHigh->GetWidth();

                ConstructPatchFeatureVector(data->BHigh, data->BLow, dstRow, dstCol, f0);

                //Propagate(data, dstPatch, i, (n % 2) == 1);
                //RandomSearch(data, dstPatch, i);
            }
        }
    }
        
    void ImageAnalogy::normalizeFeature(double* f, int nDims) 
    {
        double sum = 0.0f;

        for (int i = 0; i < nDims; i++) {
            sum += f[i];
        }

        for (int i = 0; i < nDims; i++) {
            f[i] /= sum;
        }
    }
    

    double ImageAnalogy::computeFeatureDistance(double* f1, double* f2, int nDims) 
    {
        double distance = 0.0f;
        double d;

        for (int i = 0; i < nDims; i++) {
            d = fabs(f1[i] - f2[i]);
            distance += d*d;
        }

        return sqrt(distance);
    }

    float ImageAnalogy::computeFeatureDistance(const std::vector<float>& f1, const std::vector<float>& f2)
    {
        assert(f1.size() == f2.size());
        float distance = 0.0f;
        int n = 0;

        for (UINT i = 0; i < f1.size(); i++)
        {
            if (f1[i] >= 0 &&f2[i] >= 0)
            {
                float d = f1[i] - f2[i];
                distance += d*d;
                n++;
            }
        }

        return distance / n;
    }

    void ImageAnalogy::postProcess(const Image*, const Image* APrime, const Image* B, Image* BPrime) 
    {
        switch (m_postProcessType) {
            case POSTPROCESS_APRIME_COLORS:
                postProcessRGB(APrime, BPrime);					//No-op
                break;
            case POSTPROCESS_B_COLORS:
                postProcessLuminance(B, BPrime);
                break;
        }
    }

    void ImageAnalogy::postProcessRGB(const Image*, Image*) 
    {
        //Default behavior from algorithm
    }

    void ImageAnalogy::postProcessLuminance(const Image* src, Image* result) 
    {
        UINT w = result->GetWidth();
        UINT h = result->GetHeight();
        
        for (UINT row = 0; row < h; row++) 
        {
            for (UINT col = 0; col < w; col++) 
            {
                Color4f pixel = src->GetPixel(row, col);
                FLOAT y, i, q;

                Color4f::RGBtoYIQ(pixel.r, pixel.g, pixel.b, y, i, q);

                y = result->GetPixel(row, col).ToLuminance();

                Color4f::YIQtoRGB(y, i, q, pixel.r, pixel.g, pixel.b);
                pixel.ClampRGB();

                result->SetPixel(pixel, row, col);
            }
        }
    }

    /* Does luminance remapping
     *    A[i] = sigmaB/sigmaA * (A[i] - mA) + mB
     */
    void ImageAnalogy::remapLuminance(Image* A, const Image* B) 
    {
        FLOAT meanA, stddevA;
        FLOAT meanB, stddevB;

        Color4f meanColorA, stddevColorA;
        Color4f meanColorB, stddevColorB;

        ImageMetrics::ComputeMeanAndStdDeviation(A, meanColorA, stddevColorA);
        ImageMetrics::ComputeMeanAndStdDeviation(B, meanColorB, stddevColorB);

        meanA = meanColorA.ToLuminance();
        meanB = meanColorB.ToLuminance();
        stddevA = stddevColorA.ToLuminance();
        stddevB = stddevColorB.ToLuminance();

        float stdRatio = stddevB / stddevA;

        for (UINT r = 0; r < A->GetHeight(); r++) 
        {
            for (UINT c = 0; c < A->GetWidth(); c++) 
            {
                Color4f pixel = A->GetPixel(r, c);
                FLOAT luminance = pixel.ToLuminance();
                luminance = stdRatio * (luminance - meanA) + meanB;               
                luminance = CLAMP(luminance, 0, 1);
                pixel.SetLuminance(luminance);
                A->SetPixel(pixel, r, c);
            }
        }
    }

    // Default to 5x5 for high and 3x3 for low
    // Out of region values will return -1
    // layout is 25 pixels for high and then 9 pixels for low
    void ImageAnalogy::ConstructPatchFeatureVector(const Image* high, const Image* low, UINT row, UINT col, std::vector<float>& result)
    {
        int index = 0;

        UINT hW = high->GetWidth();
        UINT hH = high->GetHeight();
        UINT lW = low->GetWidth();
        UINT lH = low->GetHeight();

        for (int r = (int)(row - 2); r <= (int)(row + 2); r++)
        {
            for (int c = (int)(col - 2); c <= (int)(col + 2); c++)
            {
                if (r >= 0 && r < (int)hH && c >= 0 && c < (int)hW)
                {
                    const Color4f& pixel = high->GetPixel(row, col);
                    if (m_featureType == FEATURE_LUMINANCE)
                    {
                        result[index++] = pixel.ToLuminance();
                    }
                    else if (m_featureType == FEATURE_RGB)
                    {
                        result[index++] = pixel.r;
                        result[index++] = pixel.g;
                        result[index++] = pixel.b;
                    }
                }
                else
                {
                    if (m_featureType == FEATURE_LUMINANCE)
                    {
                        result[index++] = -1;
                    }
                    else if (m_featureType == FEATURE_RGB)
                    {
                        result[index++] = -1;
                        result[index++] = -1;
                        result[index++] = -1;
                    }
                }
            }
        }

        if (low != nullptr)
        {
            row /= 2;
            col /= 2;
            for (int r = (int)(row - 1); r <= (int)(row + 1); r++)
            {
                for (int c = (int)(col - 1); c <= (int)(col + 1); c++)
                {
                    if (r >= 0 && r < (int)lH && c >= 0 && c < (int)lW)
                    {
                        const Color4f& pixel = low->GetPixel(row, col);
                        if (m_featureType == FEATURE_LUMINANCE)
                        {
                            result[index++] = pixel.ToLuminance();
                        }
                        else if (m_featureType == FEATURE_RGB)
                        {
                            result[index++] = pixel.r;
                            result[index++] = pixel.g;
                            result[index++] = pixel.b;
                        }
                    }
                    else
                    {
                        if (m_featureType == FEATURE_LUMINANCE)
                        {
                            result[index++] = -1;
                        }
                        else if (m_featureType == FEATURE_RGB)
                        {
                            result[index++] = -1;
                            result[index++] = -1;
                            result[index++] = -1;
                        }
                    }
                }
            }
        }
        else
        {
            for (int i = 0; i < 3*3; i++)
            {
                if (m_featureType == FEATURE_LUMINANCE)
                {
                    result[index++] = -1;
                }
                else if (m_featureType == FEATURE_RGB)
                {
                    result[index++] = -1;
                    result[index++] = -1;
                    result[index++] = -1;
                }
            }
        }
    }

    /* Settings for algorithm */
    void ImageAnalogy::setPyramidLevels(int levels) 
    {
        m_levels = levels;
    }
    void ImageAnalogy::setLuminanceRemapping(bool mapping) 
    {
        m_luminanceRemapping = mapping;
    }
    void ImageAnalogy::setFeatureType(FEATURE_TYPE type) 
    {
        m_featureType = type;
    }
    void ImageAnalogy::setANNError(FLOAT error) 
    {
        m_error = error;
    }
    void ImageAnalogy::setCoherence(FLOAT value) 
    {
        m_coherence = value;
    }
    void ImageAnalogy::setSaveIntermediate(bool value) 
    {
        m_saveIntermediate = value;
    }
    void ImageAnalogy::setPostProcess(POSTPROCESS_TYPE type) 
    {
        m_postProcessType = type;
    }

    /* Utility t combine results */
    Image* ImageAnalogy::combineImages(const Image* A, const Image* Aprime, const Image* B, const Image* Bprime, int border) 
    {
        UINT width = MAX(A->GetWidth(), B->GetWidth());
        UINT height = MAX(A->GetHeight(), B->GetHeight());
        
        Image* result = new Image(width*2 + 3*border, height*2 + 3*border);
        
        BltArgs blt;
        blt.nCols = 0;
        blt.nRows = 0;
        blt.srcCol = 0;
        blt.srcRow = 0;

        blt.dstCol = width/2 - A->GetWidth()/2 + border;
        blt.dstRow = height/2 - A->GetHeight()/2 + border;
        result->Blt(A, &blt);


        blt.dstRow = height/2 - Aprime->GetHeight()/2 + border;
        blt.dstCol = width/2 - Aprime->GetWidth()/2 + border*2 + width;
        result->Blt(Aprime, &blt);

        blt.dstRow = height/2 - B->GetHeight()/2 + border*2 + height;
        blt.dstCol = width/2 - B->GetWidth()/2 + border;
        result->Blt(B, &blt);

        blt.dstRow = height/2 - Bprime->GetHeight()/2 + border*2 + height;
        blt.dstCol = width/2 - Bprime->GetWidth()/2 + border*2 + width;
        result->Blt(Bprime, &blt);

        return result;
    }
}
