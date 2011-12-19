#include "ImageDiffer.h"
#include "UIHelper.h"

namespace Apollo {

ImageDiffer::ImageDiffer() {
    m_ref           = nullptr;
    m_target        = nullptr;
    m_diffData      = nullptr;
    m_displayData   = nullptr;

    m_mode          = LUMINANCE_PERCENT;
    m_magnify       = 1.0f;
    m_threshold     = 0.0f;

    m_displayDirty = false;
}

ImageDiffer::~ImageDiffer() {
    SAFE_DELETE(m_diffData);
    SAFE_DELETE(m_displayData);
}

bool ImageDiffer::Diff(const Image& ref, const Image& target) {
    SAFE_DELETE(m_diffData);
    SAFE_DELETE(m_displayData);

    if (ref.GetWidth() != target.GetWidth() ||
        ref.GetHeight() != target.GetHeight()) {
        ApolloException::NotYetImplemented();
    }

    m_ref = &ref;
    m_target = &target;
    m_diffData = new Image(ref.GetWidth(), ref.GetHeight());
    m_displayData = new Image(ref.GetWidth(), ref.GetHeight());

    bool identical = true;

    for (UINT32 row = 0; row < ref.GetHeight(); row++) {
        for (UINT32 col = 0; col < ref.GetWidth(); col++) {
            Color4f diff = ref.GetPixel(row, col) - target.GetPixel(row, col);
            diff.Abs();
            m_diffData->SetPixel(diff, row, col);

            diff *= m_magnify;
            if (diff.MaxChannel() > m_threshold) {
                Color4f c1 = ref.GetPixel(row, col);
                Color4f c2 = target.GetPixel(row, col);
                identical = false;
            }
        }
    }

    m_displayDirty = true;

    return identical;
}

void ImageDiffer::SetDiffMode(ImageDiffMode mode) {
    if (m_mode != mode) {
        m_mode          = mode;
        m_displayDirty  = true;   
    }    
}

void ImageDiffer::SetMagnification(FLOAT m) {
    if (m_magnify != m) {
		m_magnify       = m;
        m_displayDirty  = true;
    }
}

void ImageDiffer::SetThreshold(float t) {
    if (m_threshold != t) {
		m_threshold     = t;
        m_displayDirty  = true;
    }
}

const Image* ImageDiffer::GetDiffData() {
    if (m_displayDirty) UpdateDisplay();
    return m_displayData;
}

void ImageDiffer::UpdateDisplay() {
    if (m_diffData == nullptr) return;
    switch (m_mode) {
    case LUMINANCE_PERCENT:
        ComputePercentDiff();
        break;
    }

    m_displayDirty = false;
}

void ImageDiffer::ComputePercentDiff() {
    for (UINT32 row = 0; row < m_diffData->GetHeight(); row++) {
        for (UINT32 col = 0; col < m_diffData->GetWidth(); col++) {
            Color4f color = m_diffData->GetPixel(row, col);
            color.r = fabs(color.r);
            color.g = fabs(color.g);
            color.b = fabs(color.b);
            color.a = fabs(color.a);

            FLOAT diffLuminance = color.ToLuminance();
            FLOAT refLuminance = MAX(
                                    m_ref->GetPixel(row, col).ToLuminance(),
                                    m_target->GetPixel(row, col).ToLuminance());

            Color4f displayColor;       
            FLOAT diff = 0;
            if (diffLuminance > EPSILON) {
                diff = diffLuminance / refLuminance;
                diff *= m_magnify;
                if (diff <= m_threshold) diff = 0;
            }

            displayColor = UIHelper::GetColorIntensity(diff);
            m_displayData->SetPixel(displayColor, row, col);
        }
    }
}
/*
void ImageDiffer::computeABSDifference() {
    for (int i = 0; i < m_width*m_height; i++) {

		float r = fabs(m_diffData[i].r);
		float g = fabs(m_diffData[i].g);
		float b = fabs(m_diffData[i].b);

		r = r < m_threshold ? 0.0f : r*m_magnify;
		g = g < m_threshold ? 0.0f : g*m_magnify;
		b = b < m_threshold ? 0.0f : b*m_magnify;
		
		m_displayData[i].r = (unsigned char)(r * 255);
		m_displayData[i].g = (unsigned char)(g * 255);
		m_displayData[i].b = (unsigned char)(b * 255);
		m_displayData[i].a = 255;
    }
}

void ImageDiffer::computeErrorColorDifference(Color4 c) {
    for (int i = 0; i < m_width*m_height; i++) {

		float r = fabs(m_diffData[i].r) * m_magnify;
		float g = fabs(m_diffData[i].g) * m_magnify;
		float b = fabs(m_diffData[i].b) * m_magnify;

		if (r > m_threshold || g > m_threshold || b > m_threshold) {
			m_displayData[i] = c;
		}
    }
}
*/
}
