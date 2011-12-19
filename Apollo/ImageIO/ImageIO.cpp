#include <wincodec.h>
#include <atlbase.h>
#include <stdio.h>
#include "ImageIO.h"
#include "Macros.h"
#include "Image.h"

using namespace std;

namespace Apollo {

static IWICImagingFactory*	s_pImageFactory;
static BOOL					s_IsInitialized;

void ImageIO::Init() {
	if (s_IsInitialized) return;

	HRESULT hr;
	hr = ::CoInitialize(nullptr);
	
	if (SUCCEEDED(hr)) {
		hr = ::CoCreateInstance(
			CLSID_WICImagingFactory, 
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			(void**) &s_pImageFactory
			);
	}

	if (FAILED(hr)) throw ApolloImageException("Could not initialize WIC.");
	s_IsInitialized = true;
}

void ImageIO::Destroy() {
	if (s_pImageFactory) {
		s_pImageFactory->Release();
		s_pImageFactory = nullptr;
	}
	s_IsInitialized = false;
}

static REFGUID GetEncoderType(const char* ext) {
    if (!strcmp(ext, "png")) {
        return GUID_ContainerFormatPng;
    } else if (!strcmp(ext, "tiff")) {
        return GUID_ContainerFormatTiff;
    } else {
        throw ApolloImageException("Unsupported image type.");
    }
    return GUID_ContainerFormatPng;
}

void ImageIO::Save(const char* pPath, ImageData* pImageData) {
    if (!s_IsInitialized) throw ApolloImageException("Image IO component not initialized.");
    if (pPath == nullptr || pImageData == nullptr) throw ApolloImageException("Invalid Parameters.");

    const char* extRaw = strrchr(pPath, '.') + 1;
    char ext[256];
    UINT32 extLen = 0;
    while (*extRaw) {
        ext[extLen++] = *extRaw;
        extRaw++;
    }
    ext[extLen] = '\0';

    if (!strcmp(ext, "ppm")) {
	    FILE* file;
	    fopen_s(&file, pPath, "w");
        if (!file) throw ApolloImageException("Could not save file.");
	    fprintf(file, "P3\n");
	    fprintf(file, "%d %d\n", pImageData->uiWidth, pImageData->uiHeight);
	    fprintf(file, "%d\n", 255);
	    BYTE* pData = pImageData->pByteData;

	    for (UINT row = 0; row < pImageData->uiHeight; row++) {
		    for (UINT col = 0; col < pImageData->uiWidth; col++) {
			    fprintf(file, "%d %d %d ", pData[0], pData[1], pData[2]);
			    pData+=4;
		    }
		    fprintf(file, "\n");
	    }
	    fclose(file);
    } else {
        char buffer[1000];
        GetCurrentDirectory(1000, buffer);

        CComPtr<IWICBitmapEncoder>          pEncoder;
        CComPtr<IWICStream>                 pStream;
        CComPtr<IWICBitmapFrameEncode>      pFrame;
        GUID pixelFormat                    = GUID_WICPixelFormat32bppPBGRA;
        
        UINT32  w = pImageData->uiWidth, h = pImageData->uiHeight;

        UINT len	        = strlen(pPath) + 1;
        auto_ptr<wchar_t>   pWText(new wchar_t[len]);
        ::MultiByteToWideChar( CP_ACP, 0, pPath, -1, pWText.get(), len);

        HRESULT hr;
        
        hr = s_pImageFactory->CreateStream(&pStream);
        if (FAILED(hr)) throw ApolloImageException("Could not save image.");
        hr = pStream->InitializeFromFilename(pWText.get(), GENERIC_WRITE);        
        if (FAILED(hr)) throw ApolloImageException("Could not create file.");
        
        hr = s_pImageFactory->CreateEncoder(GetEncoderType(ext), nullptr, &pEncoder);
        if (FAILED(hr)) throw ApolloImageException("Could not save image.");      
        hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);        
        if (FAILED(hr)) throw ApolloImageException("Could not save image.");
        hr = pEncoder->CreateNewFrame(&pFrame, nullptr);
        if (FAILED(hr)) throw ApolloImageException("Could not save image.");
        
        hr = pFrame->Initialize(nullptr);
        if (FAILED(hr)) throw ApolloImageException("Could not save image.");
        hr = pFrame->SetSize(w, h);
        if (FAILED(hr)) throw ApolloImageException("Could not save image.");
        hr = pFrame->SetPixelFormat(&pixelFormat);
        if (FAILED(hr)) throw ApolloImageException("Could not save image.");

        auto_ptr<BYTE> pByteData(new BYTE[4*w*h]);
        BYTE* pData = pByteData.get();
        memcpy(pData, pImageData->pByteData, 4*w*h);

        // Loop through the bytes and convert from BGRA -> RGBA
        for (UINT i = 0; i < w*h; i++) {
	        BYTE temp = pData[0];
	        pData[0] = pData[2];
	        pData[2] = temp;
	        pData += 4;
        }

        hr = pFrame->WritePixels(pImageData->uiHeight, 4*w, 4*w*h, pByteData.get());
        if (FAILED(hr)) throw ApolloImageException("Could not save image.");
        hr = pFrame->Commit();
        if (FAILED(hr)) throw ApolloImageException("Could not save image.");
        hr = pEncoder->Commit(); 
        if (FAILED(hr)) throw ApolloImageException("Could not save image.");
    }
}

void ImageIO::Save(const char* pPath, const Image* image) {
    if (!s_IsInitialized) throw ApolloImageException("Image IO component not initialized.");
    if (pPath == nullptr || image == nullptr) throw ApolloImageException("Invalid Parameters.");

    auto_ptr<BYTE> data(new BYTE[4 * image->GetWidth() * image->GetHeight()]);

    ImageData imageData;
    imageData.uiWidth = image->GetWidth();
    imageData.uiHeight = image->GetHeight();
    imageData.format = Apollo::IMAGE_DATA_FORMAT_BYTE;
    imageData.pByteData = data.get();
    image->CopyData(imageData.pByteData, true);

    Apollo::ImageIO::Save(pPath, &imageData);
}

void ImageIO::Load(const char* pPath, ImageData* pImageData) {
	CComPtr<IWICBitmapDecoder>		pDecoder			= nullptr;
	CComPtr<IWICBitmapFrameDecode>	pFrame				= nullptr;
	CComPtr<IWICBitmapSource>		pBitmap				= nullptr;
	CComPtr<IWICFormatConverter>	pFormatConverter	= nullptr;

	HRESULT					hr;
	UINT					width, height;

	if (!s_IsInitialized) throw ApolloImageException("Image IO component not initialized.");
	if (!pPath || !pImageData) throw ApolloImageException("Invalid parameters.");

	UINT len	= strlen(pPath) + 1;
    auto_ptr<wchar_t> pWText(new wchar_t[len]);
	::MultiByteToWideChar( CP_ACP, 0, pPath, -1, pWText.get(), len);

	hr = s_pImageFactory->CreateDecoderFromFilename(
		pWText.get(), 
		nullptr, 
		GENERIC_READ, 
		WICDecodeMetadataCacheOnDemand,
		&pDecoder);
	if (FAILED(hr)) throw ApolloImageException("Could not decode file.", hr);

	if (FAILED(hr = pDecoder->GetFrame(0, &pFrame))) {
		throw ApolloImageException("Could not decode image.", hr);
	}
	if (FAILED(hr = pFrame->QueryInterface(IID_IWICBitmapSource, (void**) &pBitmap))) {
		throw ApolloImageException("Could not decode image.", hr);
	}
	if (FAILED(hr = s_pImageFactory->CreateFormatConverter(&pFormatConverter))) {
		throw ApolloImageException("Could not convert image.", hr);
	}

	hr = pFormatConverter->Initialize(
		pBitmap,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.f,
		WICBitmapPaletteTypeCustom
		);
	if (FAILED(hr)) throw ApolloImageException("Could not convert image.", hr);

	if (FAILED(hr = pFormatConverter->GetSize(&width, &height))) {
		throw ApolloImageException("Could not decode image.", hr);
	}
	
    auto_ptr<BYTE> pByte(new BYTE[width * height * 4]);
	
	hr = pFormatConverter->CopyPixels(nullptr, width * sizeof(BYTE) * 4, width* height * sizeof(BYTE) * 4, pByte.get());
	if (FAILED(hr)) throw ApolloImageException("Could not parse image.", hr);

	pImageData->uiWidth		= width;
	pImageData->uiHeight	= height;
	pImageData->format		= IMAGE_DATA_FORMAT_BYTE;
    pImageData->pByteData	= pByte.release();

    BYTE* pData = pImageData->pByteData;
	// Loop through the bytes and convert from BGRA -> RGBA
	for (UINT i = 0; i < width * height; i++) {
		BYTE temp = pData[0];
		pData[0] = pData[2];
		pData[2] = temp;
		pData += 4;
	}
}

Image* ImageIO::Load(const char* pPath) {
    ImageData imageData;
    ImageIO::Load(pPath, &imageData);
    Image* image = new Image(&imageData);
    ImageIO::Deallocate(&imageData);
    return image;
}

void ImageIO::Deallocate(ImageData* pImageData) {
	if (pImageData) {
		SAFE_ARRAY_DELETE(pImageData->pData);
	}
}

}
