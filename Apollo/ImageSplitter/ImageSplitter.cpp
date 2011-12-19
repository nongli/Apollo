#include "ApolloCommon.h"
#include "Image.h"
#include "ImageIO.h"

using namespace Apollo;
using namespace std;

void PrintUsage()
{
    printf("Usage: ImageSplitter.exe <Image> <MaxWidth> <MaxHeight>\n");
}

void SaveImage(const Image* image, const char* outputDir, const char* name)
{
    string path = outputDir;
    path.append(name);
    ImageIO::Save(path.c_str(), image);
}

void SplitImage(const Image* image, const char* outputDir, int maxW, int maxH)
{
    UINT w = image->GetWidth();
    UINT h = image->GetHeight();

    UINT nTilesW = (UINT)ceil(w / (double)maxW);
    UINT nTilesH = (UINT)ceil(h / (double)maxH);

    int num = 0;

    for (UINT tH = 0; tH < nTilesH; tH++) {
        for (UINT tW = 0; tW < nTilesW; tW++) {
            int x = (int)(tW * maxW);
            int y = (int)(tH * maxH);
            int wp = MIN(maxW, (int)(w - x));
            int hp = MIN(maxH, (int)(h - y));

            Image* partial = image->Crop(x, y, wp, hp);

            char buf[100];
            sprintf_s(buf, "Result_Part%d.png", num);
            num++;

            SaveImage(partial, outputDir, buf);
            delete partial;
        }
    }
}

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        PrintUsage();
    }
    else
    {
        char* imagePath = argv[1];
        int maxW = atoi(argv[2]);
        int maxH = atoi(argv[3]);

        /*
        char* imagePath = "B:\\Data\\colorize.png";
        int maxW = 300;
        int maxH = 300;
        */
        ImageIO::Init();

        Image* image = Apollo::ImageIO::Load(imagePath);
        SplitImage(image, ".\\", maxW, maxH);
    }
    return 0;
}