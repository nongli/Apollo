#include "ImageTests.h"

using namespace Apollo;

class CircularImage {
public:
    CircularImage(Image* image, UINT32 cX, UINT32 cY, FLOAT maxRadius, FLOAT minRadius=0) :
            m_image(image),
            m_cX(cX),
            m_cY(cY),
            m_maxRadius(maxRadius),
            m_minRadius(minRadius) {
      }

    Image* ToImage() const {
        UINT32 size = (UINT32)(m_maxRadius * 2);
        FLOAT rMax2 = m_maxRadius * m_maxRadius;
        FLOAT rMin2 = m_minRadius * m_minRadius;

        Image* image = new Image(size, size);

        UINT32 rowStart = (int)(m_cY - m_maxRadius);
        UINT32 colStart = (int)(m_cX - m_maxRadius);

        Color4f* dst = image->GetData();
        Color4f* src = m_image->GetData();

        for (UINT32 r = 0; r < size; r++) {
            for (UINT32 c = 0; c < size; c++) {
                FLOAT dy = r - m_maxRadius;
                FLOAT dx = c - m_maxRadius;

                UINT32 srcRow = rowStart + r;
                UINT32 srcCol = colStart + c;

                FLOAT d2 = dx*dx + dy*dy;

                if (d2 < rMax2 && d2 >= rMin2 && IS_IN_INTERVAL(srcRow, 0, m_image->GetHeight()) && IS_IN_INTERVAL(srcCol, 0, m_image->GetWidth())) {
                    dst[r * image->GetPitch() + c] = src[(rowStart + r) * m_image->GetPitch() + (colStart + c)];
                } else {
                    dst[r * image->GetPitch() + c] = Color4f::ZERO();
                }
            }
        }
        return image;
    }

    CircularImage* Scale(FLOAT newMaxRadius, FLOAT newMinRadius=0) const {
        UINT32 originalSize = (UINT32)(m_maxRadius * 2);
        FLOAT newMaxRadius2 = newMaxRadius * newMaxRadius;
        FLOAT newMinRadius2 = newMinRadius * newMinRadius;
        UINT32 newSize = (int)(newMaxRadius * 2);

        Image* dstImage = new Image(newSize, newSize);
        Image* srcImage = ToImage();

        CircularImage* result = new CircularImage(dstImage, originalSize, originalSize, newMaxRadius, newMinRadius);

        Color4f* src = srcImage->GetData();
        Color4f* dst = dstImage->GetData();

        for (UINT32 dstRow = 0; dstRow < newSize; dstRow++) {
            for (UINT32 dstCol = 0; dstCol < newSize; dstCol++) {
                FLOAT dy = dstRow - newMaxRadius;
                FLOAT dx = dstCol - newMaxRadius;

                FLOAT d2 = dx*dx + dy*dy;

                if (d2 < newMaxRadius2 && d2 >= newMinRadius2) {
                    // Convert dx, dy to polar coordinates
                    FLOAT d = sqrt(d2);
                    FLOAT radiusFraction = (d - newMinRadius) / (newMaxRadius - newMinRadius);
                    FLOAT theta = atan2(dy, dx);
                    
                    assert(IS_IN_INTERVAL(radiusFraction, 0, 1)); 
                    
                    FLOAT srcRadius = radiusFraction * (m_maxRadius - m_minRadius) + m_minRadius;
                    FLOAT srcDX = srcRadius * cos(theta);
                    FLOAT srcDY = srcRadius * sin(theta);

                    UINT32 srcRow = (UINT32)(srcDY + m_maxRadius);
                    UINT32 srcCol = (UINT32)(srcDX + m_maxRadius);

                    dst[dstRow * dstImage->GetPitch() + dstCol] = src[srcRow * srcImage->GetPitch() + srcCol];
                } else {
                    dst[dstRow * dstImage->GetPitch() + dstCol] = Color4f::ZERO();
                }
            }
        }

        return result;
    }

private:
    Image*  m_image;
    UINT32  m_cX;  
    UINT32  m_cY;
    FLOAT   m_maxRadius;
    FLOAT   m_minRadius;
};

bool CircularImageTest::Execute() {
    m_timer.Start();

    Color4f color;

    Image* lena = ApolloTestFramework::LoadImage("Lena.png", ApolloTestFramework::DATA_IMAGE);
    VALIDATE_NOT_NULL(lena);
    m_allocator.Add(lena);

    CircularImage image(lena, 200, 200, 100);
    CircularImage* cImage = image.Scale(200);
    m_allocator.Add(cImage);
    
    m_timer.Stop();

    ApolloTestFramework::SaveImage(this, image.ToImage(), "Before.png", ApolloTestFramework::RESULT_MISC);
    ApolloTestFramework::SaveImage(this, cImage->ToImage(), "After.png", ApolloTestFramework::RESULT_MISC);

    Image* checker = Image::GenerateCheckerboard(512, 512, 32);
    m_allocator.Add(checker);
    CircularImage c2(lena, 200, 200, 100, 50);
    CircularImage* c2Scale = c2.Scale(200, 150);
    m_allocator.Add(c2Scale);
    ApolloTestFramework::SaveImage(this, c2.ToImage(), "BeforeDonut.png", ApolloTestFramework::RESULT_MISC);
    ApolloTestFramework::SaveImage(this, c2Scale->ToImage(), "AfterDonut.png", ApolloTestFramework::RESULT_MISC);
    
    return true;
}
