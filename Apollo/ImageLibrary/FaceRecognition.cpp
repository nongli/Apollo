#include "FaceRecognition.h"
#include "FaceDetection.h"

static const char* haar_frontface = "B:\\Data\\OpenCV\\data\\haarcascades\\haarcascade_frontalface_default.xml";
static const int faceWidth = 100;
static const int faceHeight = 100;

namespace Apollo
{
    FaceRecognition::FaceRecognition(FaceRecognition::TrainingData& trainingData)
    {
        m_trainingData = &trainingData;
    }
    
    LuminanceImage<unsigned char>* FaceRecognition::ReconstructImage(const LuminanceImage<float>* image) const
    {
        cv::Mat data(1, image->GetSize(), CV_32FC1, (void*)(image->GetData()));

        data = data - m_trainingData->m_pca->mean;
        Mat result, newImage;
        m_trainingData->m_pca->project(data, result);
        m_trainingData->m_pca->backProject(result, newImage);
        LuminanceImage<unsigned char>* resultImage = new LuminanceImage<unsigned char>(image->GetWidth(), image->GetHeight());
        memcpy(resultImage->GetData(), newImage.data, sizeof(unsigned char) * image->GetSize());
        return resultImage;
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
            data[i] = newV;
        }
    }

    void FaceRecognition::TrainingData::PerformPCA(std::vector<LuminanceImage<float>*>& images)
    {
        // Put all the images into one big matrix where each image is a row
        int size = images[0]->GetSize();
        std::vector<float> matrix;
        matrix.resize(images.size() * size);

        for (size_t i = 0; i < images.size(); i++)
        {
            memcpy(&matrix[0] + i*size, images[i]->GetData(), sizeof(float) * size);
        }

        Mat data((int)images.size(), size, CV_32FC1, &matrix[0]);
        m_pca = new PCA(data, Mat(), CV_PCA_DATA_AS_ROW, s_nEigenValues);
        
        Mat meanImage = m_pca->mean.reshape(1, 100);
        imwrite("B:\\data\\eigenfaces\\mean.jpg", meanImage);

        printf("%d\n", m_pca->eigenvectors.type());
        printf("%d\n", m_pca->eigenvectors.channels());

        for (int i = 0; i < m_pca->eigenvectors.rows; i++)
        {
            for (int j = 0; j < m_pca->eigenvectors.cols; j++)
            {
                m_pca->eigenvectors.at<float>(i, j) *= 255*255;
            }
            Mat row = m_pca->eigenvectors.row(i).reshape(1, 100);
            char buf[1024];
            sprintf(buf, "B:\\data\\eigenfaces\\eigenface%d.png", i + 1);
            imwrite(buf, row);
        }
    }

    FaceRecognition::TrainingData::~TrainingData()
    {
        if (m_pca)
        {
            delete m_pca;
        }
    }

    FaceRecognition::TrainingData::TrainingData(std::vector<LuminanceImage<float>*>& images)
    {
        m_pca = NULL;
        PerformPCA(images);
    }

    FaceRecognition::TrainingData::TrainingData(int options, std::vector<SimpleImage<unsigned char>*> images)
    {
        m_pca = NULL;

        FaceDetection detection;
        detection.AddCascadeClassifier(FaceDetection::HaarFrontalDefault, haar_frontface);

        vector<SimpleImage<unsigned char>*> faceImages;
        for (int i = 0; i < images.size(); i++)
        {
            if (options & Options::RemoveAffineTransforms)
            {
                std::vector<FaceDetection::Face> faces;
                detection.FindFaces(images[i], faces);

                if (faces.size() == 0)
                {
                    throw FaceRecognitionException("No faces found on input image");
                }


                for (int j = 0; j < faces.size(); j++)
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
            for (int i = 0; i < faceImages.size(); i++)
            {
                processedImages.push_back(faceImages[i]->ToLuminanceImage<unsigned char>());
                delete faceImages[i];
            }
            faceImages.clear();

            if (options & Options::HistogramEqualize)
            {
                for (int i = 0; i < processedImages.size(); i++)
                {
                    HistogramEqualize(processedImages[i]);
                }
            }


            for (int i = 0; i < processedImages.size(); i++)
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
