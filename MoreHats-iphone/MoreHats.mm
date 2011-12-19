#include "MoreHats.h"
#include <math.h>
#import <Foundation/Foundation.h>
// PANDA
//const char* haar_frontface = "B:\\Data\\OpenCV\\data\\haarcascades\\haarcascade_frontalface_default.xml";
const NSString* HAAR_FRONTFACE_FILE = @"haarcascade_frontalface_default";
const NSString* HAAR_FRONTFACE_TYPE = @"xml";

MoreHats::Frame::Frame(const MoreHats* hats, SimpleImage<unsigned char>* image)
{
    m_moreHats = hats;
    m_srcImage = image;
}

int MoreHats::Frame::NumFaces() const
{
    return m_faces.size();
}

Point2D MoreHats::Frame::GetFaceAt(int index)
{
    Point2D center;
    center.row = m_faces[index].row;
    center.col = m_faces[index].col;
    return center;
}

string MoreHats::Frame::GetHatAt(int index)
{
    return m_hats[index];
}

void MoreHats::Frame::SetHatAt(int index, string hat)
{
    m_hats[index] = hat;
}

void MoreHats::Frame::RandomHats()
{
    for (int i = 0; i < m_faces.size(); i++)
    {
        m_hats[i] = m_moreHats->RandomHat();
    }
}


MoreHats::MoreHats(const char* haarFilePath) : m_haarFilePath(haarFilePath)
{
    NSString *haarFile = [[NSBundle mainBundle]
                          pathForResource:HAAR_FRONTFACE_FILE
                          ofType:HAAR_FRONTFACE_FILE];
    // Init face recognition
    m_faceRecognition = new FaceDetection();
    m_faceRecognition->AddCascadeClassifier(FaceDetection::HaarFrontalDefault, m_haarFilePath);
}

MoreHats::~MoreHats()
{
    map<string, Hat>::iterator iter;
    for (iter = m_hats.begin(); iter != m_hats.end(); iter++)
    {
        delete iter->second.bitmap;
    }
    delete m_faceRecognition;
}

void MoreHats::AddHat(string name, const Hat& image)
{
    m_hats[name] = image;
}

string MoreHats::RandomHat() const
{
    int r = rand() % m_hats.size();
    map<string, Hat>::const_iterator iter;
    for (iter = m_hats.begin(); iter != m_hats.end(); iter++, r--)
    {
        if (r == 0)
        {
            return iter->first;
        }
    }
    return "";
}

MoreHats::Frame* MoreHats::ProcessImage(SimpleImage<unsigned char>* image) const
{
    Frame* frame = new Frame(this, image);
    vector<FaceDetection::Face> faces;
    m_faceRecognition->FindFaces(image, faces);
    for (UINT i = 0; i < faces.size(); i++)
    {
        frame->m_faces.push_back(faces[i]);
        frame->m_hats.push_back("");
    }
    return frame;
}

SimpleImage<unsigned char>* MoreHats::ComposeFrame(const MoreHats::Frame* frame)
{
    SimpleImage<unsigned char>* image = new SimpleImage<unsigned char>(frame->m_srcImage);

    for (int i = 0; i < frame->m_faces.size(); i++)
    {
        const FaceDetection::Face& face = frame->m_faces[i];
        string hatName = frame->m_hats[i];
        if (hatName == "")
        {
            continue;
        }
        const Hat& hat = m_hats[hatName];
        
        const SimpleImage<unsigned char>* hatBitmap = hat.bitmap;
        int hatX = (hat.rightX + hat.leftX) / 2;
        int hatY = hat.y;

        SimpleImage<unsigned char>* resizedHat = NULL;

        int hatWidth = hat.rightX - hat.leftX;
        float scale = (face.w) / (float)hatWidth;
        
        if (fabs(scale - 1.0f) > .1)
        {
            int newW = (int)(scale * hat.bitmap->GetWidth());
            int newH = (int)(scale * hat.bitmap->GetHeight());
            resizedHat = hat.bitmap->Resize(newW, newH);
            hatBitmap = resizedHat;
            hatX *= scale;
            hatY *= scale;
        }

        int headCenterX = face.col;
        int headCenterY = face.row - face.h / 2;
        image->AlphaBlend(hatBitmap, headCenterY - hatY, headCenterX - hatX);

        if (resizedHat != NULL)
        {
            delete resizedHat;
        }
    }
    return image;
}
