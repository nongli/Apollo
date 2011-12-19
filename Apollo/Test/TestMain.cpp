#include "ApolloCommon.h"
#include "ApolloMatrix.h"
#include "ApolloPCA.h"

#include "FaceRecognition.h"
#include "PatchMatch.h"
#include "MoreHats.h"

#include <iostream>
#include <fstream>
using namespace std;

#ifdef UNREFERENCED_PARAMETER
#undef UNREFERENCED_PARAMETER
#endif
#include <Windows.h>

void PCATest()
{
    Apollo::Matrix<DOUBLE> data(10, 3);
    for (UINT j = 0; j < data.NumRows(); j++)
    {
        for (UINT i = 0; i < data.NumCols(); i++)
        {
            data.SetData(j, i, 1.0);
        }
    }

    //PCA* pca = PCA::ComputePCA(data);
}

void MatrixTest()
{
    Apollo::FixedMatrix<int, 3, 4> m;
    m.SetData(1, 1, 5);
    printf("%d\n", m.GetData(1, 1));

    Apollo::Matrix<double> m2(10, 10);
    m2.SetData(1, 1, 5.0);
    printf("%f\n", m2.GetData(1, 1));
}

void Hats()
{
    Apollo::ImageIO::Init();
    srand((unsigned int)time(0));

    File root = "E:\\Data\\Hats";
    FileSystem::TraverseFolder(root);

    MoreHats moreHats;
    std::vector<File> files;
    root.AppendAllLeafChildren(files);
    
    for (std::vector<File>::iterator iter = files.begin(); iter != files.end(); iter++)
    {
        Image* image = Apollo::ImageIO::Load(&(iter->GetFullPath()[0]));
        MoreHats::Hat sombreror = {image->ToSimpleImage<unsigned char>(), image->GetWidth() / 4, image->GetWidth() * 3 / 4, image->GetHeight() / 2};
        moreHats.AddHat(iter->GetFileName(), sombreror);
        delete image;
    }
    /*
    char* sombrerorfile = "B:\\Data\\sombrero.png";
    char* fedorafile = "B:\\Data\\fedora.png";
    
    // Leaks the Apollo::Image but w/e
    MoreHats::Hat sombreror = {Apollo::ImageIO::Load(sombrerorfile)->ToSimpleImage<unsigned char>(), 50, 90, 60};
    MoreHats::Hat fedora = {Apollo::ImageIO::Load(fedorafile)->ToSimpleImage<unsigned char>(), 12, 94, 56};
    
    MoreHats moreHats;
    moreHats.AddHat("sombreror", sombreror);
    moreHats.AddHat("fedora", fedora);
    */

    char* file1 = "E:\\Data\\JessicaAlba.jpg";
    char* file2 = "E:\\Data\\baywatch.jpg";
    char* file3 = "E:\\Data\\pandalian.jpg";
    char* file4 = "E:\\Data\\itay.png";
    Apollo::Image* image1 = Apollo::ImageIO::Load(file1);
    Apollo::Image* image2 = Apollo::ImageIO::Load(file2);
    Apollo::Image* image3 = Apollo::ImageIO::Load(file3);
    Apollo::Image* image4 = Apollo::ImageIO::Load(file4);

    MoreHats::Frame* frame1 = moreHats.ProcessImage(image1->ToSimpleImage<unsigned char>());
    MoreHats::Frame* frame2 = moreHats.ProcessImage(image2->ToSimpleImage<unsigned char>());
    MoreHats::Frame* frame3 = moreHats.ProcessImage(image3->ToSimpleImage<unsigned char>());
    MoreHats::Frame* frame4 = moreHats.ProcessImage(image4->ToSimpleImage<unsigned char>());

    delete image1;
    delete image2;

    frame1->RandomHats();
    frame2->RandomHats();
    frame3->RandomHats();
    frame4->RandomHats();

    SimpleImage<unsigned char>* result1 = moreHats.ComposeFrame(frame1);
    SimpleImage<unsigned char>* result2 = moreHats.ComposeFrame(frame2);
    SimpleImage<unsigned char>* result3 = moreHats.ComposeFrame(frame3);
    SimpleImage<unsigned char>* result4 = moreHats.ComposeFrame(frame4);

    delete frame1;
    delete frame2;

    Apollo::ImageIO::Save("E:\\data\\out1.png", result1->ToImage());
    Apollo::ImageIO::Save("E:\\data\\out2.png", result2->ToImage());
    Apollo::ImageIO::Save("E:\\data\\out3.png", result3->ToImage());
    Apollo::ImageIO::Save("E:\\data\\out4.png", result4->ToImage());

    delete result1;
    delete result2;
}

void PrintMatch(int n, int match[], float error[])
{
    for (int i = 0; i < n; i++)
    {
        printf("%d(%0.2f),  ", match[i], error[i]);
    }
    printf("\n");
}

void PrintMatch(int n, int match[], float error[], std::vector<string>& files)
{
    for (int i = 0; i < n; i++)
    {
        printf("   %s(%0.2f)\n", files[match[i]].c_str(), error[i]);
    }
}

struct FaceDataInput
{
    int id;
    vector<string> images;
};

void TraverseFaces(const char* directory, std::map<string, FaceDataInput>& result, const char* lastDir=nullptr)
{
    std::vector<char> path(1024);
    sprintf_s(&path[0], 1024, "%s\\*", directory);

    WIN32_FIND_DATA info;
    HANDLE dir = FindFirstFile(&path[0], &info);
    if (dir != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(strcmp(info.cFileName, ".") == 0 || strcmp(info.cFileName, "..") == 0))
            {
                sprintf_s(&path[0], 1024, "%s\\%s", directory, info.cFileName);
                if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    TraverseFaces(&path[0], result, info.cFileName);
                }
                else
                {
                    result[lastDir].images.push_back(&path[0]);
                }
            }
        } while (FindNextFile(dir, &info));
        FindClose(dir);
    }
}

void TraverseDirectory(const char* directory, std::vector<string>& files)
{
    std::vector<char> path(1024);
    sprintf_s(&path[0], 1024, "%s\\*", directory);

    WIN32_FIND_DATA info;
    HANDLE dir = FindFirstFile(&path[0], &info);
    if (dir != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(strcmp(info.cFileName, ".") == 0 || strcmp(info.cFileName, "..") == 0))
            {
                sprintf_s(&path[0], 1024, "%s\\%s", directory, info.cFileName);
                if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    TraverseDirectory(&path[0], files);
                }
                else
                {
                    files.push_back(&path[0]);
                }
            }
        } while (FindNextFile(dir, &info));
        FindClose(dir);
    }
}

FaceRecognition::TrainingData* Train(std::string directory)
{
    std::map<string, FaceDataInput> faces;
    TraverseFaces(directory.c_str(), faces);

    std::vector<SimpleImage<unsigned char>*> rawFaces;
    int id = 0;
    std::map<string, FaceDataInput>::iterator iter;
    for (iter = faces.begin(); iter != faces.end(); iter++)
    {
        iter->second.id = id++;
        for (unsigned int i = 0; i < iter->second.images.size(); i++)
        {
            Apollo::Image* image = Apollo::ImageIO::Load(iter->second.images[i].c_str());
            rawFaces.push_back(image->ToSimpleImage<unsigned char>());
            delete image;
        }
    }
    FaceRecognition::TrainingData* data = new FaceRecognition::TrainingData(        
            FaceRecognition::Options::RemoveAffineTransforms |
            FaceRecognition::Options::Grayscale |
            FaceRecognition::Options::HistogramEqualize, 
            rawFaces);

    return data;
}

void TrainTest()
{
    Apollo::ImageIO::Init();
    FaceRecognition::TrainingData* data = Train("D:\\Faces");
}

void Faces()
{
    Apollo::ImageIO::Init();

    /*
    std::vector<SimpleImage<unsigned char>*> input;
    for (int i = 1;  i <= 50; i++)
    {
        char buf[1024];
        sprintf(buf, "B:\\data\\maxim100\\%03d.jpg", i);
        Apollo::Image* image = Apollo::ImageIO::Load(buf);
        input.push_back(image->ToSimpleImage<unsigned char>());
        delete image;
    }
    FaceRecognition::TrainingData data(
        FaceRecognition::Options::RemoveAffineTransforms |
        FaceRecognition::Options::Grayscale |
        FaceRecognition::Options::HistogramEqualize, 
        input);
    */
    std::vector<string> files;
    TraverseDirectory("B:\\data\\FacesMaxim", files);
    std::vector<LuminanceImage<float>*> input;
    for (unsigned int i = 0; i < files.size(); i++)
    {
        Apollo::Image* image = Apollo::ImageIO::Load(files[i].c_str());
        input.push_back(image->ToLuminanceImage<float>());
        delete image;
    }

    FaceRecognition::TrainingData data(input);
    FaceRecognition recognizer(data);


    LuminanceImage<float>* mean = recognizer.GetMeanFace();
    Apollo::ImageIO::Save("B:\\data\\meanface.png", mean->ToImage());

    for (unsigned int i = 0; i < files.size(); i++)
    {
        recognizer.AddFaceToDB(input[i], i);
    }

    /*
    for (unsigned int i = 0; i < input.size(); i++)
    {
        LuminanceImage<float>* reconstructed = recognizer.ReconstructImage(input[i]);
        Image* image = reconstructed->ToImage();
        char buf[1024];
        sprintf(buf, "B:\\data\\Reconstructed\\Image%d.png", i+1);
        ImageIO::Save(buf, image);
        delete image;
        delete reconstructed;
    }
    */

    float totalError = 0;
    float error[5];
    int match[5];
    int nResults = 5;
    /*
    for (unsigned int i = 0; i < input.size(); i++)
    {
        recognizer.MatchFace(input[i], match, error, nResults);
        totalError += error[0];
        printf("Matched %s -> ", files[i].c_str());
        PrintMatch(nResults, match, error);
    }
    printf("Total Error: %f\n", totalError);
    */
    LuminanceImage<float>* test1 = recognizer.FindAndNormalizeFace(Apollo::ImageIO::Load("B:\\data\\Jessica_Alba_cropped.png")->ToSimpleImage<unsigned char>());
    recognizer.MatchFace(test1, match, error, nResults);
    printf("Matched Jessica Alba -> ");
    PrintMatch(nResults, match, error, files);

    LuminanceImage<float>* test2 = recognizer.FindAndNormalizeFace(Apollo::ImageIO::Load("B:\\data\\Tatty.jpg")->ToSimpleImage<unsigned char>());
    Apollo::ImageIO::Save("B:\\data\\cropped.png", test2->ToImage());
    recognizer.MatchFace(test2, match, error, nResults);
    printf("Matched Tatty -> ");
    PrintMatch(nResults, match, error, files);

    LuminanceImage<float>* test3 = recognizer.FindAndNormalizeFace(Apollo::ImageIO::Load("B:\\data\\Lian.jpg")->ToSimpleImage<unsigned char>());
    recognizer.MatchFace(test3, match, error, nResults);
    printf("Matched Lian -> ");
    PrintMatch(nResults, match, error, files);

    LuminanceImage<float>* test4 = recognizer.FindAndNormalizeFace(Apollo::ImageIO::Load("B:\\data\\Mike.jpg")->ToSimpleImage<unsigned char>());
    recognizer.MatchFace(test4, match, error, nResults);
    printf("Matched Mike -> ");
    PrintMatch(nResults, match, error, files);

    LuminanceImage<float>* test5 = recognizer.FindAndNormalizeFace(Apollo::ImageIO::Load("B:\\data\\Mike2.jpg")->ToSimpleImage<unsigned char>());
    recognizer.MatchFace(test5, match, error, nResults);
    printf("Matched Mike2 -> ");
    PrintMatch(nResults, match, error, files);

    LuminanceImage<float>* test6 = recognizer.FindAndNormalizeFace(Apollo::ImageIO::Load("B:\\data\\Mike3.jpg")->ToSimpleImage<unsigned char>());
    recognizer.MatchFace(test6, match, error, nResults);
    printf("Matched Mike3 -> ");
    PrintMatch(nResults, match, error, files);
}

void PatchMatchTest()
{
    Apollo::ImageIO::Init();
    Apollo::Timer::Init();
    srand((unsigned int)time(0));
    Apollo::Image* tree = Apollo::ImageIO::Load("E:\\Data\\tree.jpg");
    Apollo::Image* clouds = Apollo::ImageIO::Load("E:\\Data\\clouds.jpg");
    
    Apollo::Image* testImage = clouds;

    Timer timer;
    Apollo::Image* output = new Apollo::Image(*testImage);
    //Rect<int> region(100, 100, 150, 100);
    Rect<int> region(126, 228, 143, 108);
    timer.Start();
    PatchMatch::ComputePatchMatch(testImage, output, &region, PatchMatch::RGB_L2, 5);
    timer.Stop();
    printf("Time Elapsed (ms) %d\n", timer.GetTimeElapsed());
    Apollo::ImageIO::Save("E:\\Data\\result.png", output);
}

void TextDetection()
{
    Apollo::ImageIO::Init();
    Apollo::Timer::Init();

	Apollo::Image* input = Apollo::ImageIO::Load("E:\\Data\\Camera.png");

	CannyEdgeDetect edgeDetect(input);
	Image* output = edgeDetect.GetEdgeImage();

	Apollo::ImageIO::Save("E:\\Data\\output.png", output);
}

class Object
{
public:
    Object()
    {
        printf("  Created: %p\n", this);
    }
    ~Object()
    {
        printf("Destroyed: %p\n", this);
    }

    void Test()
    {
        printf("%d\n", m_x);
    }

private:
    int m_x;
};

std::shared_ptr<Object> ToSharedPointer(Object* o)
{
    return shared_ptr<Object>(o);
}

std::shared_ptr<Object> CreateInstance()
{
    return std::shared_ptr<Object>(new Object);
}

std::shared_ptr<Object> CreateInstance2()
{
    std::shared_ptr<Object> o(new Object);
    return o;
}

void SharedPointerTest()
{
    Object o1;
    std::shared_ptr<Object> o2(new Object);
    Object* o3 = new Object;
    std::shared_ptr<Object> o3s = ToSharedPointer(o3);

    shared_ptr<Object> o4 = CreateInstance();
    shared_ptr<Object> o5 = CreateInstance2();

    o1.Test();
    o2->Test();
    o3->Test();
    o4->Test();
    o5->Test();
}

void FileSystemTest()
{
    Apollo::File file = File::Load("E:\\Data\\Hats");
    FileSystem::TraverseFolder(file);

    std::vector<File> files;
    file.AppendAllLeafChildren(files);
    
    for (std::vector<File>::iterator iter = files.begin(); iter != files.end(); iter++)
    {
        printf("%s\n", iter->GetFullPath().c_str());
    }
}

int main(int argc, char** argv)
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    //MatrixTest();
    //PCATest();
    Hats();
    //Faces();
    //TrainTest();
    //PatchMatchTest();

//    TextDetection();
//    SharedPointerTest();
//    FileSystemTest();

    printf("Done.\n");
    getchar();
}