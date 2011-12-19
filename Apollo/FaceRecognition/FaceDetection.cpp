#include "FaceDetection.h"
#ifdef UNREFERENCED_PARAMETER
#undef UNREFERENCED_PARAMETER
#endif

#pragma warning(push )
#pragma warning( disable:4996 ) // Potentially unsafe string handling in opencv
#include "opencv2\opencv.hpp"
#pragma warning( pop )

using namespace cv;

namespace Apollo
{
    FaceDetection::FaceDetection()
    {
        Ptr<FeatureEvaluator> object = FeatureEvaluator::create(FeatureEvaluator::HAAR);
        object.addref();
        m_evaluator = object;
    }

    FaceDetection::~FaceDetection()
    {
        std::map<ClassifierType, cv::CascadeClassifier*>::iterator iter;
        for (iter = m_classifiers.begin(); iter != m_classifiers.end(); iter++)
        {
            delete iter->second;
        }
        if (m_evaluator != nullptr)
        {
            delete m_evaluator;
        }
    }

    void FaceDetection::AddCascadeClassifier(ClassifierType type, const std::string& path)
    {
        cv::CascadeClassifier* classifier = new cv::CascadeClassifier(path);
        if (classifier == nullptr || classifier->empty())
        {
            throw FaceDetectionException("Invalid classifier.");
        }
        m_classifiers[type] = classifier;
    }

    bool FaceDetection::FindFaces(const SimpleImage<unsigned char>* image, std::vector<Face>& faces)
    {
        if (m_classifiers.size() == 0)
        {
            throw FaceDetectionException("Must add classifiers before using face recognition.");
        }

        const SimpleImage<unsigned char>::Pixel* data = image->GetData();
        vector<cv::Rect> features;
        cv::Mat input(image->GetHeight(), image->GetWidth(), CV_8UC4, (void*)data);
        CascadeClassifier* classifier = m_classifiers[HaarFrontalDefault];
        cv::Ptr<cv::FeatureEvaluator> evaluator(m_evaluator);
        evaluator.addref();
        classifier->setImage(evaluator, input);
        classifier->detectMultiScale(input, features);

        for (UINT i = 0; i < features.size(); i++)
        {
            Face face;
            face.w = features[i].width;
            face.h = features[i].height;
            face.col = face.w / 2 + features[i].x;
            face.row = face.h / 2 + features[i].y;
            faces.push_back(face);
        }
        return features.size() > 0;
    }

    bool FaceDetection::FindFaces(const Image* image, std::vector<Face>& faces)
    {
        if (m_classifiers.size() == 0)
        {
            throw FaceDetectionException("Must add classifiers before using face recognition.");
        }

        vector<BYTE> data;
        data.resize(image->GetSize() * 3);
        image->CopyData(&data[0], false, true);

        vector<cv::Rect> features;
        cv::Mat input((int)image->GetHeight(), (int)image->GetWidth(), CV_8UC3, &data[0]);
        CascadeClassifier* classifier = m_classifiers[HaarFrontalDefault];
        cv::Ptr<cv::FeatureEvaluator> evaluator(m_evaluator);
        evaluator.addref();
        classifier->setImage(evaluator, input);
        classifier->detectMultiScale(input, features);

        for (UINT i = 0; i < features.size(); i++)
        {
            Face face;
            face.w = features[i].width;
            face.h = features[i].height;
            face.col = face.w / 2 + features[i].x;
            face.row = face.h / 2 + features[i].y;
            faces.push_back(face);
        }
        return features.size() > 0;
    }
}