#pragma once

#include "ApolloCommon.h"
#include "Texture2D.h"

namespace Apollo {
    template <typename T>
    class TextureSampler {
    public:
        T           value;
        Texture2D*  texture2D;

        TextureSampler(const T& v) : value(v), texture2D(nullptr) {}
        TextureSampler(Texture2D* texture) : texture2D(texture) {}

        inline void GetSample(const UV&, T&) const {
            throw ApolloException("This should not be called.");
        }
    };

    //
    // Sampler to return color4fs
    //
    void TextureSampler<Color4f>::GetSample(const UV& uv, Color4f& c) const {
        if (texture2D) texture2D->GetSample(uv, c);
        else c = value;
    }

    //
    // Sampler to return FLOATs
    //
    void TextureSampler<FLOAT>::GetSample(const UV& uv, FLOAT& f) const {
        if (texture2D) {
            Color4f c;
            texture2D->GetSample(uv, c);
            f = c.ToLuminance();
        }
        else {
            f = value;
        }    
    }

    //
    // Sampler to return UINT32s
    //
    void TextureSampler<UINT32>::GetSample(const UV& uv, UINT32& ui) const {
        ApolloException::NotYetImplemented();
        UNREFERENCED_PARAMETER(uv);
        UNREFERENCED_PARAMETER(ui);
    }
}
