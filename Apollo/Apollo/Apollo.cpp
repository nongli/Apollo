#include <stdio.h>
#include "ImageIO.h"
#include "Image.h"
#include "Timer.h"
#include "Error.h"
#include "UIHelper.h"
#include "ImageDiffer.h"
#include "ApolloEngine.h"
#include "Vector3.h"

using namespace Apollo;
using namespace std;

int AATest() {
    printf("Starting AA Tests.\n");
	SamplerUtil::Init(false);

    
	Timer timer;	
	Image image(640, 480);
	PerspectiveCamera camera;
	camera.SetImageDimension(640, 480);
	camera.SetAspectRatio(((FLOAT)640)/480);
	camera.SetPosition(Vector3(0, 0, 10));	
	
    Sphere sphere1(Vector3(2, 0, 0), 2.5);
    Sphere sphere2(Vector3(-2, 0, 0), 2.5);

	sphere1.Init();
    sphere2.Init();

	Vector3 light(0, 10, 13);

	LinearAccel accel;
	accel.AddGeometry(&sphere1);
    accel.AddGeometry(&sphere2);
	accel.Init();
	
    int* superSamples = new int[640 * 480];
    memset(superSamples, 0, 640 * 480 * sizeof(int));

	timer.Start();

	Ray				ray;
	Ray             shadow;
    Intersection	intersection;
	SurfaceElement	surfel;
    
    ScanLineBucket scanLineBucket(640, 480);
    
    StochasticSuperSampler primarySampler(512);
    //GridSuperSampler primarySampler(23);
    //PrimarySampler primarySampler;
    //AdaptiveSuperSampler primarySampler;

    primarySampler.SetRenderBucket(&scanLineBucket);
    primarySampler.Reset(0, 0, 480, 640);
    primarySampler.SetPixelSize(1.8f);

    PrimarySample sample(&image);

    while (primarySampler.GetNextSample(sample)) {
        superSamples[sample.row * 640 + sample.col]++;
        camera.GenerateRay(ray, sample.xFilm, sample.yFilm);
        intersection.Reset();

		if (accel.Intersect(ray, intersection)) {
            sample.model = intersection.primitive->GetParentModel();
			surfel.Init(&intersection, &ray);

			Vector3 lightVec(light - surfel.iPoint);
			shadow.Set(surfel.iPoint, lightVec);

			if (accel.Intersect(shadow) == NO_HIT) {
                DOUBLE ln = shadow.direction.Dot(surfel.normal);
				ln = fabs(ln);
				if (ln > 0) {
                    FLOAT lnf = (FLOAT)ln;
                    if (intersection.primitive == &sphere1) {
                        sample.SetColor(Color4f(lnf, 0, 0));					    
                    } else {
                        sample.SetColor(Color4f(0, 0, lnf));
                    }
                }
			}
		} else {
            sample.model = nullptr;
            sample.SetColor(Color4f::ZERO());
        }
    }    
    printf("Time to render image: %d(ms)\n", timer.Stop());

    int minSamples = 3000;
    int maxSamples = 0;
    int totalSamples = 0;
    for (int i = 0; i < 640 * 480; i++) {
        minSamples = MIN(minSamples, superSamples[i]);
        maxSamples = MAX(maxSamples, superSamples[i]);
        totalSamples += superSamples[i];
    }
    printf("Min Samples: %d\n", minSamples);
    printf("Max Samples: %d\n", maxSamples);
    printf("Total Samples: %d\n", totalSamples);
    printf("Averages Samples per pixel: %f\n", (FLOAT)totalSamples / (640 * 480));
    Image samples(640, 480);
    for (int row = 0; row < 480; row++) {
        for (int col = 0; col < 640; col++) {
            int s = superSamples[row * 640 + col];
            FLOAT value = (s - minSamples) / (FLOAT)(maxSamples - minSamples);
            Color4f color = UIHelper::GetColorIntensity(value);
            samples.SetPixel(color, row, col);
        }
    }

    ImageIO::Save("..\\Results\\AASamples.png", &samples);
	ImageIO::Save("..\\Results\\AARender.png", &image);

    printf("End AA Tests.\n");
    
    return 0;
}

void GenerateSnowFlake(int depth, vector<Model*>& model, Sphere* sphere) {
	model.push_back(sphere);
	if (depth == 0) return;

	DOUBLE locoffset	= sphere->radius / 2 + sphere->radius;
	FLOAT size			= sphere->radius / 2;
	Vector3 loc	= sphere->center;

	GenerateSnowFlake(depth - 1, model, new Sphere(Vector3(loc.x, loc.y + locoffset, loc.z), size));
	GenerateSnowFlake(depth - 1, model, new Sphere(Vector3(loc.x, -(loc.y + locoffset), loc.z), size));

	GenerateSnowFlake(depth - 1, model, new Sphere(Vector3(loc.x + locoffset, loc.y, loc.z), size));
	GenerateSnowFlake(depth - 1, model, new Sphere(Vector3(-(loc.x + locoffset), loc.y, loc.z), size));

	GenerateSnowFlake(depth - 1, model, new Sphere(Vector3(loc.x, loc.y, loc.z + locoffset), size));
	GenerateSnowFlake(depth - 1, model, new Sphere(Vector3(loc.x, loc.y, -(loc.z + locoffset)), size));
}

int KochFlake() {
	printf("Starting SnowFlake Tests.\n");
	SamplerUtil::Init(false);

	Timer timer;	
	Image image(640, 480);
	PerspectiveCamera camera(Vector3(5, 0, 5), Vector3(-5, 0, -5));
	camera.SetImageDimension(640, 480);
	camera.SetAspectRatio(((FLOAT)640)/480);
	
	Sphere sphere;
	Vector3 light(0, 0, 10);

	vector<Model*> spheres;
	GenerateSnowFlake(4, spheres, new Sphere());

	LinearAccel accel;
	accel.SetGeometry(spheres);
	accel.Init();
			
	timer.Start();
	Ray				ray;
	Ray             shadow;
    Intersection	intersection;
	SurfaceElement	surfel;
    
    ScanLineBucket scanLineBucket(640, 480);
	PrimarySampler primarySampler;
    primarySampler.SetPixelSize(2.0f);
    primarySampler.SetRenderBucket(&scanLineBucket);
    primarySampler.Reset(0, 0, 480, 640);

    PrimarySample sample(&image);

    while (primarySampler.GetNextSample(sample)) {
        camera.GenerateRay(ray, sample.xFilm, sample.yFilm);
        intersection.Reset();

		if (accel.Intersect(ray, intersection)) {
			surfel.Init(&intersection, &ray);

			Vector3 lightVec(light - surfel.iPoint);
			shadow.Set(surfel.iPoint, lightVec);

			//if (accel.Intersect(shadow) == NO_HIT)
			{
                DOUBLE ln = shadow.direction.Dot(surfel.normal);
				ln = fabs(ln);
				if (ln > 0) {
                    FLOAT lnf = (FLOAT)ln;
                    sample.SetColor(Color4f(lnf, lnf, lnf));					    
                }
			}
		}
    }

	printf("Time to render image: %d(ms)\n", timer.Stop());
	ImageIO::Save("..\\Results\\Kochflake.png", &image);

	return 0;
}

int main(int, char**) {
    try {
	    ImageIO::Init();
	    Timer::Init();

		Image* image = Image::GenerateGrid(1024, 1024, 7, 6);
		ImageIO::Save("..\\Data\\Grid.png", image);
    } catch (exception& e) {        
		printf("EXCEPTION: %s\n", e.what());
    }

	char c;
	scanf_s("%c", &c);
}