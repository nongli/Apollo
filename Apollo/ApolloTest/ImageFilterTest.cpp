#include "ImageTests.h"

using namespace Apollo;

bool ImageFilterTest::Execute() {    
    m_timer.Start();

    FilterKernel kernel2D, kernel1D;
    FilterKernel::CreateTriangleFilter2D(&kernel2D, 11);
    FilterKernel::CreateTriangleFilter1D(&kernel1D, 11);

    Image* image = ApolloTestFramework::LoadImage("Lighthouse.png", ApolloTestFramework::DATA_IMAGE);
    VALIDATE_NOT_NULL(image);
    m_allocator.Add(image);

    Image* blurred2D = image->Convolve(&kernel2D, Apollo::Image::IMAGE_FILTER_EDGE_IGNORE);
    VALIDATE_NOT_NULL(blurred2D);
    m_allocator.Add(blurred2D);
    
    Image* blurred1D = image->ConvolveSeperated(&kernel1D, Apollo::Image::IMAGE_FILTER_EDGE_IGNORE);
    VALIDATE_NOT_NULL(blurred1D);
    m_allocator.Add(blurred1D);

    Image* mipMapped = image->GenerateMipMap();
    VALIDATE_NOT_NULL(mipMapped);
    m_allocator.Add(mipMapped);

    image->ToGrayscale(image);

    m_timer.Stop();

    bool passed = true;
    passed &= ApolloTestFramework::Instance()->ProcessResult(this, blurred2D, "LightHouseBlurred2D.png");
    passed &= ApolloTestFramework::Instance()->ProcessResult(this, blurred1D, "LightHouseBlurred1D.png");
    passed &= ApolloTestFramework::Instance()->ProcessResult(this, mipMapped, "LightHouseMipmapped.png");
    passed &= ApolloTestFramework::Instance()->ProcessResult(this, image, "LightHouseGrayscale.png");

    return passed;
}
