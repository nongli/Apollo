#include "FaceRecognition.h"
#include "FaceDetection.h"
#ifdef UNREFERENCED_PARAMETER
#undef UNREFERENCED_PARAMETER
#endif
#pragma warning(push )
#pragma warning( disable:4996 ) // Potentially unsafe string handling in opencv
#include "opencv2\opencv.hpp"
#pragma warning( pop )
#include <queue>

static const char* haar_frontface = "B:\\Data\\OpenCV\\data\\haarcascades\\haarcascade_frontalface_default.xml";
static const int faceWidth = 100;
static const int faceHeight = 100;

namespace Apollo
{
    FaceRecognition::FaceRecognition(FaceRecognition::TrainingData& trainingData)
    {
        m_trainingData = &trainingData;
    }
    
    LuminanceImage<float>* FaceRecognition::ReconstructImage(const LuminanceImage<float>* image) const
    {
        cv::Mat data(1, image->GetSize(), CV_32FC1, (void*)(image->GetData()));
        cv::Scalar value = cv::sum(data);
        cv::Mat result, newImage;
        m_trainingData->m_pca->project(data, result);
        m_trainingData->m_pca->backProject(result, newImage);
        LuminanceImage<float>* resultImage = new LuminanceImage<float>(image->GetWidth(), image->GetHeight());
        memcpy(resultImage->GetData(), newImage.data, sizeof(float) * image->GetSize());
        return resultImage;
    }

    void FaceRecognition::AddFaceToDB(const LuminanceImage<float>* image, int luid)
    {
        std::vector<float> projection;
        ProjectFace(image, projection);
        m_facesDB.push_back(FaceDBEntry(projection, luid));
    }

    LuminanceImage<float>* FaceRecognition::GetMeanFace() const
    {
        cv::Mat mean = m_trainingData->m_pca->mean.reshape(100, 100);
        LuminanceImage<float>* image = new LuminanceImage<float>(100, 100);
        memcpy(image->GetData(), mean.data, sizeof(float)*100*100);
        return image;
    }

    
    void FaceRecognition::MatchFace(const LuminanceImage<float>* image, int* results, float* distances, int& nFaces, MatchMetric::Metric metric) const
    {
        struct PriorityQueueEntry
        {
            float distance;
            int luid;

            PriorityQueueEntry(float d, int l)
            {
                distance = d;
                luid = l;
            }

            // stl queue is a max heap
            bool operator< (const PriorityQueueEntry& rhs) const
            {
                return distance > rhs.distance;
            }
        };

        std::priority_queue<PriorityQueueEntry> queue;

        for (unsigned int i = 0; i < m_facesDB.size(); i++)
        {
            float distance = ComputeDistance(image, m_facesDB[i].projection, metric);
            queue.push(PriorityQueueEntry(distance, m_facesDB[i].luid));
        }

        for (int i = 0; i < nFaces; i++)
        {
            distances[i] = queue.top().distance;
            results[i] = queue.top().luid;
            queue.pop();
        }
    }

    float FaceRecognition::ComputeDistance(const LuminanceImage<float>* image, const std::vector<float>& target, MatchMetric::Metric) const
    {
        float distance = 0;
        std::vector<float> src;
        ProjectFace(image, src);

        assert(src.size() == target.size());

        for (unsigned int i = 0; i < src.size(); i++)
        {
            float delta = src[i] - target[i];
            distance += delta*delta;
        }
        return distance;
    }
    
    void FaceRecognition::ProjectFace(const LuminanceImage<float>* image, std::vector<float>& projection) const
    {
        cv::Mat data(1, image->GetSize(), CV_32FC1, (void*)(image->GetData()));
        cv::Scalar value = cv::sum(data);
        cv::Mat result, newImage;
        m_trainingData->m_pca->project(data, result);

        projection.resize(result.cols);
        float total = 0;
        for (int i = 0; i < result.cols; i++)
        {
            projection[i] = result.at<float>(0, i);
            total += projection[i] * projection[i];
        }

        total = sqrt(total);

        for (int i = 0; i < result.cols; i++)
        {
            projection[i] /= total;
        }
    }

    void FaceRecognition::TrainingData::HistogramEqualize(LuminanceImage<unsigned char>* image)
    {
        std::vector<int> buckets;
        std::vector<int> cdf;
        buckets.resize(256);
        cdf.resize(256);
        unsigned char* data = image->GetData();
        int min = 100;
        for (int i = 0; i < image->GetSize(); i++)
        {
            buckets[data[i]]++;
            if (data[i] < min)
            {
                min = data[i];
            }
        }

        cdf[0] = buckets[0];
        for (int i = 1; i < 256; i++)
        {
            cdf[i] = cdf[i - 1] + buckets[i];
        }
        assert(cdf[255] == image->GetSize());

        for (int i = 0; i < image->GetSize(); i++)
        {
            int v = data[i];
            int newV = (cdf[v] - 1) * 255 / (image->GetSize() - 1);
            assert(newV >= 0 && newV <= 255);
            data[i] = (unsigned char)newV;
        }
    }

    void FaceRecognition::TrainingData::PerformPCA(std::vector<LuminanceImage<float>*>& images)
    {
        // Put all the images into one big matrix where each image is a row
        int size = images[0]->GetSize();
        std::vector<float> matrix;
        matrix.resize(images.size() * size);

        cv::Mat data(images.size(), size, CV_32FC1);
        for (size_t i = 0; i < images.size(); i++)
        {
            memcpy(data.row(i).data, images[i]->GetData(), sizeof(float) * size);
        }

        m_pca = new cv::PCA(data, cv::Mat(), CV_PCA_DATA_AS_ROW, s_nEigenValues);
    }

    FaceRecognition::TrainingData::~TrainingData()
    {
        if (m_pca)
        {
            delete m_pca;
        }
    }

    LuminanceImage<float>* FaceRecognition::FindAndNormalizeFace(const SimpleImage<unsigned char>* input) const
    {
        return m_trainingData->FindAndNormalizeFace(input);
    }

    LuminanceImage<float>* FaceRecognition::TrainingData::FindAndNormalizeFace(const SimpleImage<unsigned char>* input) const
    {
        std::vector<FaceDetection::Face> faces;
        m_faceDetection->FindFaces(input, faces);

        if (faces.size() == 0)
        {
            return nullptr;
        }
        
        vector<SimpleImage<unsigned char>*> faceImages;
        for (unsigned int j = 0; j < faces.size(); j++)
        {
            int row = faces[j].row - faces[j].h / 2;
            int col = faces[j].col - faces[j].w / 2;
            int w = faces[j].w;
            int h = faces[j].h;
            SimpleImage<unsigned char>* cropped = input->Crop(col, row, w, h);
            faceImages.push_back(cropped->Resize(faceWidth, faceHeight));
            delete cropped;
        }

        vector<LuminanceImage<unsigned char>*> processedImages;
        for (unsigned int i = 0; i < faceImages.size(); i++)
        {
            processedImages.push_back(faceImages[i]->ToLuminanceImage<unsigned char>());
            delete faceImages[i];
        }
        faceImages.clear();

        for (unsigned int i = 0; i < processedImages.size(); i++)
        {
            HistogramEqualize(processedImages[i]);
        }
        //TODO LEAKS
        return processedImages[0]->ToLuminanceImage<float>();
    }

    FaceRecognition::TrainingData::TrainingData(std::vector<LuminanceImage<float>*>& images)
    {
        m_pca = nullptr;
        m_faceDetection = new FaceDetection();
        m_faceDetection->AddCascadeClassifier(FaceDetection::HaarFrontalDefault, haar_frontface);
        PerformPCA(images);
    }

    FaceRecognition::TrainingData::TrainingData(int options, std::vector<SimpleImage<unsigned char>*> images)
    {
        m_pca = nullptr;
        m_faceDetection = new FaceDetection();
        m_faceDetection->AddCascadeClassifier(FaceDetection::HaarFrontalDefault, haar_frontface);

        vector<SimpleImage<unsigned char>*> faceImages;
        for (unsigned int i = 0; i < images.size(); i++)
        {
            if (options & Options::RemoveAffineTransforms)
            {
                std::vector<FaceDetection::Face> faces;
                m_faceDetection->FindFaces(images[i], faces);

                if (faces.size() == 0)
                {
                    throw FaceRecognitionException("No faces found on input image");
                }


                for (unsigned int j = 0; j < faces.size(); j++)
                {
                    int row = faces[j].row - faces[j].h / 2;
                    int col = faces[j].col - faces[j].w / 2;
                    int w = faces[j].w;
                    int h = faces[j].h;
                    SimpleImage<unsigned char>* cropped = images[i]->Crop(col, row, w, h);
                    faceImages.push_back(cropped->Resize(faceWidth, faceHeight));
                    delete cropped;
                }
            }
            else
            {
                faceImages.push_back(new SimpleImage<unsigned char>(images[i]));
            }
        }

        if (options & Options::Grayscale)
        {
            vector<LuminanceImage<unsigned char>*> processedImages;
            for (unsigned int i = 0; i < faceImages.size(); i++)
            {
                processedImages.push_back(faceImages[i]->ToLuminanceImage<unsigned char>());
                delete faceImages[i];
            }
            faceImages.clear();

            if (options & Options::HistogramEqualize)
            {
                for (unsigned int i = 0; i < processedImages.size(); i++)
                {
                    HistogramEqualize(processedImages[i]);
                }
            }


            for (unsigned int i = 0; i < processedImages.size(); i++)
            {
                char buf[1024];
                sprintf(buf, "B:\\Data\\Faces\\FaceImage%d.png", i + 1);
                ImageIO::Save(buf, processedImages[i]->ToImage());
            }

            // TODO
            //PerformPCA(processedImages);
        }
        else
        {
            throw FaceRecognitionException("Only grayscale supported");
        }
    }
}
