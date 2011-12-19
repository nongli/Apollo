#pragma once

#include "ApolloCommon.h"
#include "Image.h"

namespace cv
{
    class CascadeClassifier;
    class FeatureEvaluator;
};

namespace Apollo
{
    class FaceDetection
    {
    public:
        enum ClassifierType
        {
            HaarFrontalDefault,
        };

    public:
        struct Face
        {
            UINT row;       // center of face
            UINT col;       // center of face
            UINT w;         // width of face (not rotated)
            UINT h;         // height of face (not rotated)
            //rotations
        };

    public:
        FaceDetection();
        virtual ~FaceDetection();

        void AddCascadeClassifier(ClassifierType type, const std::string& features);
        bool FindFaces(const Image* image, std::vector<Face>& faces);
        bool FindFaces(const SimpleImage<unsigned char>* image, std::vector<Face>& faces);

    private:
        FaceDetection(const FaceDetection&);    // not impl, don't want copy constructor
        FaceDetection& operator=(const FaceDetection&); // not impl, don't want assignment

    private:
        std::map<ClassifierType, cv::CascadeClassifier*> m_classifiers;
        cv::FeatureEvaluator* m_evaluator;
    };

    class FaceDetectionException : public ApolloException
    {
        public:
	        FaceDetectionException(std::string message) : ApolloException(message) {}
    };
};