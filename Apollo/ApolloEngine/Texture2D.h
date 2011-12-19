#pragma once

#include "ApolloCommon.h"

namespace Apollo {
    class Texture2D {
    public:
        Texture2D(Image* image) : m_image(image) {}
        ~Texture2D() {}

        void GetSample(const UV& uv, Color4f& c) const;

    private:
        Image*      m_image;
    };


    inline void Texture2D::GetSample(const UV& uv, Color4f& c) const {
        UINT32 row = (UINT32)(uv.v * m_image->GetHeight());
        UINT32 col = (UINT32)(uv.u * m_image->GetWidth());
		row = MIN(row, m_image->GetHeight() - 1);
		col = MIN(col, m_image->GetWidth() - 1);
        c = m_image->GetPixel(row, col);
    }
}
