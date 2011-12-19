#pragma once
#include "ApolloCommon.h"
#include "ImageIO.h"

namespace Apollo {
	struct FilterKernel {
		UINT32				width;
		UINT32				height;
		std::vector<FLOAT>	kernel;
		bool				normalized;

		FilterKernel(UINT width, UINT height) {
			this->width = width;
			this->height = height;
			kernel.resize(width * height);
			normalized = false;
		}

		FilterKernel() {
			this->width = 0;
			this->height = 0;
			normalized = false;
		}

		void Normalize();
		static void CreateTriangleFilter2D(FilterKernel* kernel, UINT32 size);
		static void CreateTriangleFilter1D(FilterKernel* kernel, UINT32 size);
        static void CreateGaussianFilter2D(FilterKernel* kernel, UINT32 size, FLOAT sigma);
	};

	struct BltArgs {
		UINT32				srcRow;
		UINT32				srcCol;
		UINT32				nRows;
		UINT32				nCols;
		UINT32				dstRow;
		UINT32				dstCol;
	};

    template <typename T>
    class LuminanceImage {
    public:
        LuminanceImage(UINT32 w, UINT32 h) {
            m_width = w;
            m_height = h;
            m_data.resize(w*h);
        }
    public:
        UINT32 GetWidth() const { return m_width; }
        UINT32 GetHeight() const { return m_height; }
        UINT32 GetSize() const { return m_width * m_height; }
        T* GetData() { return &m_data[0]; }
        const T* GetData() const { return &m_data[0]; }

        template <typename T>
        LuminanceImage<T>* ToLuminanceImage() const;

        template<>
        LuminanceImage<FLOAT>* ToLuminanceImage() const {
            LuminanceImage<FLOAT>* image = new LuminanceImage<FLOAT>(m_width, m_height);
            FLOAT* data = image->GetData();
            for (UINT32 i = 0; i < m_width * m_height; i++) {
                data[i] = (FLOAT)m_data[i];
            }
            return image;
        }

        Image* ToImage() const {
            ImageData data;
            data.format = IMAGE_DATA_FORMAT_BYTE_BGRA;
            std::vector<BYTE> imageData;
            imageData.resize(m_width * m_height * 4);
            for (UINT32 i = 0, index = 0; i < m_width * m_height; i++) {
                BYTE value = (BYTE) CLAMP(m_data[i], 0, 255);
                imageData[index++] = value;
                imageData[index++] = value;
                imageData[index++] = value;
                imageData[index++] = 255;
            }
            data.pByteData = (BYTE*)(&imageData[0]);
            data.uiWidth = m_width;
            data.uiHeight = m_height;
            return new Image(&data);
        }

    private:
        UINT32 m_width;
        UINT32 m_height;
        std::vector<T> m_data;
    };

    template <typename T>
    class SimpleImage {
    public:
        struct Pixel {
            T b, g, r, a;       // For some reason bgra seems more "standard"
        };

    public:
        SimpleImage(UINT32 w, UINT32 h) : m_width(w), m_height(h) {
            m_data.resize(w*h);
        }
        SimpleImage(const SimpleImage<T>* copy) {
            m_width = copy->GetWidth();
            m_height = copy->GetHeight();
            m_data.resize(m_width * m_height);
            memcpy(&m_data[0], &copy->m_data[0], sizeof(T) * 4 * m_width * m_height);
        }

    public:
        UINT32 GetWidth() const { return m_width; }
        UINT32 GetHeight() const { return m_height; }
        const Pixel* GetData() const { return &m_data[0]; }
        Pixel* GetData() { return &m_data[0]; }

    public:
        Image* ToImage() const {
            ImageData data;
            data.format = IMAGE_DATA_FORMAT_BYTE_BGRA;
            data.pByteData = (BYTE*)(&m_data[0]);
            data.uiWidth = m_width;
            data.uiHeight = m_height;
            return new Image(&data);
        }

        template <typename T>
        LuminanceImage<T>* ToLuminanceImage() const;

        template<>
        LuminanceImage<BYTE>* ToLuminanceImage() const {
            LuminanceImage<BYTE>* image = new LuminanceImage<BYTE>(m_width, m_height);
            BYTE* data = image->GetData();
            for (UINT32 i = 0; i < m_width * m_height; i++) {
                const Pixel& pixel = m_data[i];
                data[i] = (BYTE)CLAMP(.39 * pixel.r + .50 * pixel.g + .11 * pixel.b, 0, 255);
            }
            return image;
        }

    public:
        SimpleImage<T>* Crop(UINT32 col, UINT32 row, UINT32 w, UINT32 h) const {
            SimpleImage<T>* image = new SimpleImage<T>(w, h);
            const Pixel* srcData = GetData() + row*m_width + col;
            Pixel* dstData = image->GetData();
            for (UINT32 r = 0; r < h; r++) {
                memcpy(dstData, srcData, sizeof(Pixel) * w);
                srcData += m_width;
                dstData += w;
            }
            return image;
        }

        void AlphaBlend(const SimpleImage<T>* image, INT32 row, INT32 col) {
            const Pixel* srcData = image->GetData();
            UINT32 w = image->GetWidth();
            UINT32 h = image->GetHeight();
            UINT32 rStart = row < 0 ? -row : 0;
            UINT32 cStart = col < 0 ? -col : 0;
            UINT32 rEnd = (row + h) < m_height ? h : m_height - (rStart + row);
            UINT32 cEnd = (col + w) < m_width ? w : m_width - (cStart + col);
            
            for (UINT32 r = rStart; r < rEnd; r++) {
                UINT32 dstIndex = (r + row) * m_width + col;
                UINT32 srcIndex = r * w + cStart;
                for (UINT32 c = cStart; c < cEnd; c++, dstIndex++, srcIndex++) {
                    const Pixel& srcPixel = srcData[srcIndex];
                    FLOAT alpha = srcPixel.a / 255.0f;
                    m_data[dstIndex].r = (T)(m_data[dstIndex].r * (1 - alpha) + srcPixel.r);
                    m_data[dstIndex].g = (T)(m_data[dstIndex].g * (1 - alpha) + srcPixel.g);
                    m_data[dstIndex].b = (T)(m_data[dstIndex].b * (1 - alpha) + srcPixel.b);
                }
            }
        }

        SimpleImage<T>* Resize(UINT32 newW, UINT32 newH) const {
            SimpleImage<T>* image = new SimpleImage<T>(newW, newH);
            // Simple bilinear filter
            FLOAT xs = (FLOAT)m_width / newW;
            FLOAT ys = (FLOAT)m_height / newH;

            FLOAT fracx, fracy, ifracx, ifracy, sx, sy, l0, l1;
            int x0, x1, y0, y1;
            T c1a, c1r, c1g, c1b, c2a, c2r, c2g, c2b, c3a, c3r, c3g, c3b, c4a, c4r, c4g, c4b;
            T a = 0, r = 0, g = 0, b = 0;

            UINT32 srcIdx = 0;
            for (UINT32 y = 0; y < newH; y++) {
               for (UINT32 x = 0; x < newW; x++) {
                  sx = x * xs;
                  sy = y * ys;
                  x0 = (int)sx;
                  y0 = (int)sy;

                  // Calculate coordinates of the 4 interpolation points
                  fracx = sx - x0;
                  fracy = sy - y0;
                  ifracx = 1.0f - fracx;
                  ifracy = 1.0f - fracy;
                  x1 = x0 + 1;
                  if (x1 >= m_width) x1 = x0;
                  y1 = y0 + 1;
                  if (y1 >= m_height) y1 = y0;

                  // Read source color
                  c1a = m_data[y0 * m_width + x0].a;
                  c1r = m_data[y0 * m_width + x0].r;
                  c1g = m_data[y0 * m_width + x0].g;
                  c1b = m_data[y0 * m_width + x0].b;
                  
                  c2a = m_data[y0 * m_width + x1].a;
                  c2r = m_data[y0 * m_width + x1].r;
                  c2g = m_data[y0 * m_width + x1].g;
                  c2b = m_data[y0 * m_width + x1].b;

                  c3a = m_data[y1 * m_width + x0].a;
                  c3r = m_data[y1 * m_width + x0].r;
                  c3g = m_data[y1 * m_width + x0].g;
                  c3b = m_data[y1 * m_width + x0].b;

                  c4a = m_data[y1 * m_width + x1].a;
                  c4r = m_data[y1 * m_width + x1].r;
                  c4g = m_data[y1 * m_width + x1].g;
                  c4b = m_data[y1 * m_width + x1].b;

                  // Calculate colors
                  // Alpha
                  l0 = ifracx * c1a + fracx * c2a;
                  l1 = ifracx * c3a + fracx * c4a;
                  a = (T)(ifracy * l0 + fracy * l1);

                  if (a > 0) {
                     // Red
                     l0 = ifracx * c1r * c1a + fracx * c2r * c2a;
                     l1 = ifracx * c3r * c3a + fracx * c4r * c4a;
                     r = (T)((ifracy * l0 + fracy * l1) / a);

                     // Green
                     l0 = ifracx * c1g * c1a + fracx * c2g * c2a;
                     l1 = ifracx * c3g * c3a + fracx * c4g * c4a;
                     g = (T)((ifracy * l0 + fracy * l1) / a);

                     // Blue
                     l0 = ifracx * c1b * c1a + fracx * c2b * c2a;
                     l1 = ifracx * c3b * c3a + fracx * c4b * c4a;
                     b = (T)((ifracy * l0 + fracy * l1) / a);
                  } else {
                      r = g = b = 0;
                  }

                  image->GetData()[srcIdx].a = a;
                  image->GetData()[srcIdx].r = r;
                  image->GetData()[srcIdx].g = g;
                  image->GetData()[srcIdx].b = b;
                  srcIdx++;
               }
            }
            return image;
        }

    private:
        UINT32 m_width;
        UINT32 m_height;
        std::vector< Pixel > m_data;
    };

	class Image {
	public:
		enum IMAGE_FILTER_EDGE {
			IMAGE_FILTER_EDGE_IGNORE,
			IMAGE_FILTER_EDGE_REPEAT,
			IMAGE_FILTER_EDGE_MIRROR,
		};

	public:
		Image(UINT32 width, UINT32 height);
		Image(const Image& image);
		Image(ImageData* pData);

		~Image();

		UINT32 GetWidth() const { return m_width; }
		UINT32 GetHeight() const { return m_height; }
        UINT32 GetSize() const { return m_width * m_height; };
		UINT32 GetPitch() const { return m_pitch; }
		Color4f* GetData() { return m_data; }
		const Color4f* GetData() const { return m_data;}

		void SetPixel(const Color4f& c, UINT32 row, UINT32 col) { m_data[row*m_pitch + col] = c; }
		const Color4f& GetPixel(UINT32 row, UINT32 col) const { return m_data[row*m_pitch + col]; }
        const Color4f& GetPixel(UINT32 index) const { return m_data[index]; }

		void CopyData(BYTE* pData, bool includeAlpha, bool bgra = false) const;
		void CopyData(FLOAT* pData) const;
		void CopyData(DOUBLE* pData) const;
		void Blt(const Image* image, const BltArgs* args);
        void AlphaBlend(const Image* image, UINT row, UINT col);

        void RemapLuminance(FLOAT min, FLOAT max);

    public:
        template <typename T>
        SimpleImage<T>* ToSimpleImage() const;

        template<typename T>
        LuminanceImage<T>* ToLuminanceImage() const;

        template<>
        SimpleImage<BYTE>* ToSimpleImage() const {
            SimpleImage<BYTE>* image = new SimpleImage<BYTE>(m_width, m_height);
            const SimpleImage<BYTE>::Pixel* data = image->GetData();
            CopyData((BYTE*)data, true, true);
            return image;
        }

        template<>
        LuminanceImage<BYTE>* ToLuminanceImage() const {
            LuminanceImage<BYTE>* image = new LuminanceImage<BYTE>(m_width, m_height);
            BYTE* data = image->GetData();
            for (UINT32 i = 0; i < m_width * m_height; i++) {
                const Color4f& pixel = m_data[i];
                data[i] = (BYTE)CLAMP(pixel.ToLuminance() * 255, 0, 255);
            }
            return image;
        }

        template<>
        LuminanceImage<FLOAT>* ToLuminanceImage() const {
            LuminanceImage<FLOAT>* image = new LuminanceImage<FLOAT>(m_width, m_height);
            FLOAT* data = image->GetData();
            for (UINT32 i = 0; i < m_width * m_height; i++) {
                const Color4f& pixel = m_data[i];
                data[i] = pixel.ToLuminance() * 255;
            }
            return image;
        }

	// For all of these functions that have two versions, the version with no parameter
	// will return a newly allocated image with the filter applied to that one.
	// The version accepting a parameter will use the parameter as the destination.
	// The size of the destination must be correct.
	// To have an inplace scale, you can do image->ImageFilter(image).
	public:
		Image* ToGrayscale() const;
		Image* ToGrayscale(Image* dst) const;

		Image* Convolve(FilterKernel*, IMAGE_FILTER_EDGE) const;
		Image* Convolve(FilterKernel*, IMAGE_FILTER_EDGE, Image*) const;

		Image* ConvolveSeperated(FilterKernel*, IMAGE_FILTER_EDGE) const;
		Image* ConvolveSeperated(FilterKernel*, IMAGE_FILTER_EDGE, Image*) const;

		Image* NaiveScaleToHalf() const;
		Image* NaiveScaleToHalf(Image* dst) const;

        Image* Crop(UINT x, UINT y, UINT w, UINT h) const;
        //Image* Crop(UINT x, UINT y, UINT w, UINT h, Image*) const;


	public:
		Image* GenerateMipMap() const;

	public:
		static Image* GenerateCheckerboard(UINT32 w, UINT32 h, UINT32 t);
        static Image* GenerateGrid(UINT32 w, UINT32 h, UINT32 nLines, UINT32 thickness);

	private:
		Color4f*				m_data;
		UINT32					m_width;
		UINT32					m_height;
		UINT32					m_pitch;
	};

    class GaussianPyramid {
    public:
        GaussianPyramid(const Image* image);
        static GaussianPyramid* Create(const Image* image);
        ~GaussianPyramid();

        UINT32 GetNumLevels() const;
        const Image* GetImageAt(UINT32 level) const;
        Image* ToImage() const;

    private:
        std::vector<Image*> m_images;
    };


	class CannyEdgeDetect {
	public:
		struct EdgeDetectOperator {
			enum Operator {
				Default,
				Robers,
				Prewitt,
				Sobel,
			};
		};

        struct EdgeDirection {
            enum Direction {
		        Vertical,
		        Horizontal,
		        Angle45,
		        Angle135,
            };
	    };

		struct Edge {
			FLOAT gx, gy;
            FLOAT g;
            EdgeDirection::Direction dir;

			Edge() {
				gx = gy = 0;
			}

            Edge(FLOAT gx, FLOAT gy, FLOAT g, EdgeDirection::Direction dir) {
				this->gx = gx;
				this->gy = gy;
                this->g = g;
                this->dir = dir;
			}
		};

	public:
		CannyEdgeDetect(const Image* image, EdgeDetectOperator::Operator opertor = EdgeDetectOperator::Default);
		Image* GetEdgeImage() const;
	
	private:
        EdgeDirection::Direction ComputeEdgeDirection(FLOAT gx, FLOAT gy) const;
		void ComputeEdges(const Image* image, EdgeDetectOperator::Operator op);

	private:
		UINT32 m_width;
		UINT32 m_height;
		std::vector<Edge> m_edges;
	};

    //
    // Static class to compute image metrics
    //
    class ImageMetrics {
    public:
        static void ComputeMeanAndStdDeviation(const Image* image, Color4f& mean, Color4f& stddev);
    };
}
