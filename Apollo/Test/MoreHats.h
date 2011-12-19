#pragma once
#include <string>
#include <vector>
#include <map>
using namespace std;

#include "Image.h"
#include "FaceDetection.h"
using namespace Apollo;

struct Point2D
{
    int row;
    int col;
};

class MoreHats
{
public:
    class Frame
    {
    friend class MoreHats;

    public:
        int NumFaces() const;
        
        Point2D GetFaceAt(int index);
        string GetHatAt(int index);

        void SetHatAt(int index, string hat);
        void RandomHats();

    private:
        Frame(const MoreHats* hats, SimpleImage<unsigned char>* image);

    private:
        const MoreHats* m_moreHats;
        SimpleImage<unsigned char>* m_srcImage;
        vector<FaceDetection::Face> m_faces;                
        vector<string> m_hats;                  // hats at each face
    };

public:
    struct Hat
    {
        const SimpleImage<unsigned char>* bitmap;
        int leftX;
        int rightX;
        int y;
    };

public:
    MoreHats();
    ~MoreHats();

public:
    // Add a hat image.  This object will inherit the memory (memory should be knew-ed)
    void AddHat(string name, const Hat& hat);
    string RandomHat() const;

    // Process an image, getting back a frame object.  Process will run the expensive (face detection) part
    // Frame should be deleted by caller
    Frame* ProcessImage(SimpleImage<unsigned char>* image) const;
    // Compose the frame with hats into a single image
    // Returned image should be deleted by caller
    SimpleImage<unsigned char>* ComposeFrame(const Frame* frame);

public:
    SimpleImage<unsigned char>* GetHatImage(string name);

private:
    map<string, Hat> m_hats;
    FaceDetection* m_faceDetection;
};