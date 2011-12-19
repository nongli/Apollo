#include "Image.h"

namespace Apollo {

Image* Image::GenerateCheckerboard(UINT32 w, UINT32 h, UINT32 t)
{
	if (t == 0) ApolloImageException("Invalid parameter.");
	if (w % t != 0 || h % t != 0) ApolloException::NotYetImplemented();

	bool first = false;

	Color4f c1 = Color4f::BLACK();
	Color4f c2 = Color4f::WHITE();

	Image* image = new Image(w, h);
		
	for (UINT32 rt = 0; rt < h; rt += t) {
		first = !first;
		for (UINT32 ct = 0; ct < w; ct += t) {
			first = !first;
			for (UINT32 r = 0; r < t; r++) {
				for (UINT32 c = 0; c < t; c++) {
					UINT32 row = rt + r;
					UINT32 col = ct + c;
					image->SetPixel(first ? c1 : c2, row, col);
				}
			}
		}
	}
		
	return image;
}

Image* Image::GenerateGrid(UINT32 w, UINT32 h, UINT32 nLines, UINT32 thickness) {
	Color4f c1 = Color4f::BLACK();
	Color4f c2 = Color4f::WHITE();
        
    UINT32 nSpaces = nLines + 1;
    UINT32 wGap = w / nSpaces;
    UINT32 hGap = h / nSpaces;

    if (w % nSpaces != 0 || h % nSpaces != 0) {
        ApolloException::NotYetImplemented();
    }
    if (wGap <= thickness || hGap <= thickness) {
        ApolloException::NotYetImplemented();
    }

	Image* image = new Image(w, h);
    for (UINT32 r = 0; r < h; r++) {
        for (UINT32 c = 0; c < w; c++) {
            image->SetPixel(c1, r, c);
        }
    }


    for (UINT32 c = 1; c < nSpaces; c++) {
        for (UINT32 row = 0; row < h; row++) {
            for (INT32 t = -(INT32)thickness; t <= (INT32)thickness; t++) {
                UINT32 col = c * wGap + t;
                image->SetPixel(c2, row, col);
            }
        }
    }

    for (UINT32 r = 1; r < nSpaces; r++) {
        for (UINT32 col = 0; col < w; col++) {
            for (INT32 t = -(INT32)thickness; t <= (INT32)thickness; t++) {
                UINT32 row = r * hGap + t;
                image->SetPixel(c2, row, col);
            }
        }
    }
		
	return image;
}

}
