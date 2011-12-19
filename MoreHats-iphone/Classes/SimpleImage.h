/*
 *  SimpleImage.h
 *  MoreHats-iphone
 *
 *  Created by Aurojit Panda on 1/9/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#include "ApolloCommon.h"
#pragma once

namespace Apollo {    
template <typename T>
class SimpleImage
{
public:
    struct Pixel
    {
        T b, g, r, a;       // For some reason bgra seems more "standard"
    };
    
public:
    SimpleImage(int w, int h) : m_width(w), m_height(h)
    {
        m_data.resize(w*h);
    }
    SimpleImage(const SimpleImage<T>* copy)
    {
        m_width = copy->GetWidth();
        m_height = copy->GetHeight();
        m_data.resize(m_width * m_height);
        memcpy(&m_data[0], &copy->m_data[0], sizeof(T) * 4 * m_width * m_height);
    }
    
public:
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    const Pixel* GetData() const { return &m_data[0]; }
    Pixel* GetData() { return &m_data[0]; }
    
public:
    void AlphaBlend(const SimpleImage<T>* image, int row, int col)
    {
        int w = image->GetWidth();
        int h = image->GetHeight();
        
        const Pixel* srcData = image->GetData();
        
        int rStart = row < 0 ? -row : 0;
        int cStart = col < 0 ? -col : 0;
        int rEnd = (row + h) < m_height ? h : m_height - (rStart + row);
        int cEnd = (col + w) < m_width ? w : m_width - (cStart + col);
        
        for (int r = rStart; r < rEnd; r++)
        {
            int dstIndex = (r + row) * m_width + col;
            int srcIndex = r * w + cStart;
            
            for (int c = cStart; c < cEnd; c++, dstIndex++, srcIndex++)
            {
                const Pixel& srcPixel = srcData[srcIndex];
                float alpha = srcPixel.a / 255.0f;
                m_data[dstIndex].r = (T)(m_data[dstIndex].r * (1 - alpha) + srcPixel.r);
                m_data[dstIndex].g = (T)(m_data[dstIndex].g * (1 - alpha) + srcPixel.g);
                m_data[dstIndex].b = (T)(m_data[dstIndex].b * (1 - alpha) + srcPixel.b);
            }
        }
    }
    
    SimpleImage<T>* Resize(int newW, int newH) const
    {
        SimpleImage<T>* image = new SimpleImage<T>(newW, newH);
        
        // Simple bilinear filter
        float xs = (float)m_width / newW;
        float ys = (float)m_height / newH;
        
        float fracx, fracy, ifracx, ifracy, sx, sy, l0, l1;
        int x0, x1, y0, y1;
        T c1a, c1r, c1g, c1b, c2a, c2r, c2g, c2b, c3a, c3r, c3g, c3b, c4a, c4r, c4g, c4b;
        T a = 0, r = 0, g = 0, b = 0;
        
        int srcIdx = 0;
        for (int y = 0; y < newH; y++)
        {
            for (int x = 0; x < newW; x++)
            {
                sx = x * xs;
                sy = y * ys;
                x0 = (int)sx;
                y0 = (int)sy;
                
                // Calculate coordinates of the 4 interpolation points
                fracx = sx - x0;
                fracy = sy - y0;
                ifracx = 1.0f - fracx;
                ifracy = 1.0f - fracy;
                x1 = x0 + 1;
                if (x1 >= m_width)
                    x1 = x0;
                y1 = y0 + 1;
                if (y1 >= m_height)
                    y1 = y0;
                
                // Read source color
                c1a = m_data[y0 * m_width + x0].a;
                c1r = m_data[y0 * m_width + x0].r;
                c1g = m_data[y0 * m_width + x0].g;
                c1b = m_data[y0 * m_width + x0].b;
                
                c2a = m_data[y0 * m_width + x1].a;
                c2r = m_data[y0 * m_width + x1].r;
                c2g = m_data[y0 * m_width + x1].g;
                c2b = m_data[y0 * m_width + x1].b;
                
                c3a = m_data[y1 * m_width + x0].a;
                c3r = m_data[y1 * m_width + x0].r;
                c3g = m_data[y1 * m_width + x0].g;
                c3b = m_data[y1 * m_width + x0].b;
                
                c4a = m_data[y1 * m_width + x1].a;
                c4r = m_data[y1 * m_width + x1].r;
                c4g = m_data[y1 * m_width + x1].g;
                c4b = m_data[y1 * m_width + x1].b;
                
                // Calculate colors
                // Alpha
                l0 = ifracx * c1a + fracx * c2a;
                l1 = ifracx * c3a + fracx * c4a;
                a = (T)(ifracy * l0 + fracy * l1);
                
                if (a > 0)
                {
                    // Red
                    l0 = ifracx * c1r * c1a + fracx * c2r * c2a;
                    l1 = ifracx * c3r * c3a + fracx * c4r * c4a;
                    r = (T)((ifracy * l0 + fracy * l1) / a);
                    
                    // Green
                    l0 = ifracx * c1g * c1a + fracx * c2g * c2a;
                    l1 = ifracx * c3g * c3a + fracx * c4g * c4a;
                    g = (T)((ifracy * l0 + fracy * l1) / a);
                    
                    // Blue
                    l0 = ifracx * c1b * c1a + fracx * c2b * c2a;
                    l1 = ifracx * c3b * c3a + fracx * c4b * c4a;
                    b = (T)((ifracy * l0 + fracy * l1) / a);
                }
                else
                {
                    r = g = b = 0;
                }
                
                image->GetData()[srcIdx].a = a;
                image->GetData()[srcIdx].r = r;
                image->GetData()[srcIdx].g = g;
                image->GetData()[srcIdx].b = b;
                srcIdx++;
            }
        }
        
        return image;
    }
    
private:
    int m_width;
    int m_height;
    std::vector< Pixel > m_data;
};
}
