#pragma once

#include "Types.h"
#include "Error.h"

namespace Apollo {
    class Image;

    class ApolloImageException : public ApolloException {
    public:
	    ApolloImageException(std::string message) : ApolloException(message) {}
	    ApolloImageException(std::string message, UINT32 hr) : ApolloException() {
		    char buffer[32];
		    sprintf_s(buffer, sizeof(buffer), "0x%x", hr);
		    m_message = message + "  HResult: " + buffer;
	    }
    };

    enum ImageDataFormat {
	    IMAGE_DATA_FORMAT_BYTE,
	    IMAGE_DATA_FORMAT_FLOAT,
	    IMAGE_DATA_FORMAT_DOUBLE,
        IMAGE_DATA_FORMAT_BYTE_BGRA
    };

    struct ImageData {
	    UINT32				uiWidth;
	    UINT32				uiHeight;	
	    ImageDataFormat		format;

	    union {
		    FLOAT*			pFloatData;
		    DOUBLE*			pDoubleData;
		    BYTE*			pByteData;
		    void*			pData;
	    };

	    ImageData() {
		    uiWidth = uiHeight = 0;
		    pData	= nullptr;
	    }
    };

    class ImageIO {
    public:
	    static void Init();
	    static void Destroy() throw();

	    static void Save(const char* pPath, ImageData* pImageData);
        static void Save(const char* pPath, const Image* image);
	    static void Load(const char* pPath, ImageData* ppImageData);
        static Image* Load(const char* pPath);
	    static void Deallocate(ImageData* pImageData);
    };
}
