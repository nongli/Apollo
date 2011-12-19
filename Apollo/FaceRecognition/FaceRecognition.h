#pragma once

#include "ApolloCommon.h"
#include "Image.h"

namespace cv
{
    class PCA;
}

namespace Apollo
{
    class FaceDetection;

    class FaceRecognition
    {
    public:
        class TrainingData;

    public:
        class Options
        {
        public:
            enum ProcesingOptions
            {
                None = 0,
                RemoveAffineTransforms = 1,
                Grayscale = 2,
                HistogramEqualize = 4,
            };
        };

        class MatchMetric
        {
        public:
            enum Metric
            {
                Default = 0,
            };
        };

    public:
        FaceRecognition(TrainingData& trainingData);

    public:
        void AddFaceToDB(const LuminanceImage<float>* face, int luid);
        void MatchFace(const LuminanceImage<float>* image, int* results, float* distance, int& nResults, MatchMetric::Metric = MatchMetric::Default) const;
        LuminanceImage<float>* ReconstructImage(const LuminanceImage<float>* image) const;

        LuminanceImage<float>* GetMeanFace() const;

    public:
        LuminanceImage<float>* FindAndNormalizeFace(const SimpleImage<unsigned char>* image) const;

    private:
        void ProjectFace(const LuminanceImage<float>* image, std::vector<float>& result) const;
        float ComputeDistance(const LuminanceImage<float>* image, const std::vector<float>& projection, MatchMetric::Metric) const;

    private:
        struct FaceDBEntry
        {
            std::vector<float> projection;
            int luid;

            FaceDBEntry(const std::vector<float> p, int i)
            {
                projection = p;
                luid = i;
            }
        };

    private:
        TrainingData* m_trainingData;
        std::vector<FaceDBEntry> m_facesDB;

    public:
        class TrainingData
        {
        public:
            friend class FaceRecognition;

        public:
            TrainingData(int options, std::vector<SimpleImage<unsigned char>*> images);
            TrainingData(std::vector<LuminanceImage<float>*>& input);
            ~TrainingData();

        private:
            void PerformPCA(std::vector<LuminanceImage<float>*>& input);
            LuminanceImage<float>* FindAndNormalizeFace(const SimpleImage<unsigned char>* image) const;

        private:
            static void HistogramEqualize(LuminanceImage<unsigned char>* image);
            
        private:
            cv::PCA* m_pca;
            FaceDetection* m_faceDetection;

        private:
            static const int s_faceWidth = 100;
            static const int s_faceHeight = 100;
            static const int s_nEigenValues = 25;
        };
    };

    class FaceRecognitionException : public ApolloException
    {
        public:
	        FaceRecognitionException(std::string message) : ApolloException(message) {}
    };
}
