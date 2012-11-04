#include "ImageTests.h"

using namespace Apollo;

bool BasicImageTest::Execute() {
    m_timer.Start();

    Color4f color;

    Image* image1 = ApolloTestFramework::LoadImage("Lena.png", ApolloTestFramework::DATA_IMAGE);
    VALIDATE_NOT_NULL(image1);
    m_allocator.Add(image1);
    VALIDATE_EQUALS(image1->GetWidth(), 512);
    VALIDATE_EQUALS(image1->GetHeight(), 512);
    VALIDATE_EQUALS(image1->GetPitch(), 512);

    color = image1->GetPixel(12, 56);
    VALIDATE_EQUALS(color.r, .88235295f);
    VALIDATE_EQUALS(color.g, .43529412f);
    VALIDATE_EQUALS(color.b, .34117648f);  
    VALIDATE_EQUALS(color.a, 1);

    image1->SetPixel(color, 0, 0);
    color = image1->GetPixel(0, 0);
    VALIDATE_EQUALS(color.r, .88235295f);
    VALIDATE_EQUALS(color.g, .43529412f);
    VALIDATE_EQUALS(color.b, .34117648f);  
    VALIDATE_EQUALS(color.a, 1);

    Color4f* color2 = image1->GetData() + 12*512 + 56;
    VALIDATE_EQUALS(color.r, color2->r);
    VALIDATE_EQUALS(color.g, color2->g);
    VALIDATE_EQUALS(color.b, color2->b);  
    VALIDATE_EQUALS(color.a, color2->a);

    ApolloTestFramework::SaveImage(this, image1, "BasicImageTestCopy.png", ApolloTestFramework::RESULT_MISC);
    Image* image2 = ApolloTestFramework::LoadImage("Test1-BasicImageTestCopy.png", ApolloTestFramework::RESULT_MISC);
    m_allocator.Add(image2);
    
    VALIDATE_NOT_NULL(image2);
    VALIDATE_EQUALS(image2->GetWidth(), 512);
    VALIDATE_EQUALS(image2->GetHeight(), 512);
    VALIDATE_EQUALS(image2->GetPitch(), 512);
    
    color = image2->GetPixel(12, 56);
    VALIDATE_EQUALS(color.r, .88235295f);
    VALIDATE_EQUALS(color.g, .43529412f);
    VALIDATE_EQUALS(color.b, .34117648f);  
    VALIDATE_EQUALS(color.a, 1);

    VALIDATE_TRUE(ApolloTestFramework::AreImagesIdentical(image1, image2));
    
    m_timer.Stop();
    return true;
}

