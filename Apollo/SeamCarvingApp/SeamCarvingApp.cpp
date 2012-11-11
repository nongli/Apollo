#include <windows.h>
#include "SeamCarvingApp.h"
#include "Image.h"
using namespace Apollo;

int WINAPI WinMain (HINSTANCE hThis, HINSTANCE hPrev, LPSTR lpszArg, int nCShow)
{
    try {
        Timer::Init();
        ImageIO::Init();
        Image* image = ImageIO::Load("d:\\data\\beach.png");
        SeamCarvingApp app(image->ToSimpleImage<BYTE>());
        delete image;
	    WaitForSingleObject(app.getThread(), INFINITE);	
    } catch (const ApolloException& e) {
        ILogger::Logger()->Error("Seam Carving Error: %s.", e.what());
    }
	
    return 0;	
}