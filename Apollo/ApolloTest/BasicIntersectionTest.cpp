#include "RenderTests.h"

using namespace Apollo;

BasicIntersectionTest::BasicIntersectionTest() : ApolloTest() {
}

void BasicIntersectionTest::Init() {
    ImageIO::Init();
    Timer::Init();
    SamplerUtil::Init(false);
}

bool BasicIntersectionTest::Execute() {    
    Image image(640, 480);

    m_timer.Start();
    PerspectiveCamera camera;
	camera.SetImageDimension(640, 480);
	camera.SetAspectRatio(((FLOAT)640)/480);
	camera.SetPosition(Apollo::Vector3(0, 0, 10));	
	
    Sphere sphere(Apollo::Vector3(2, 1, 0), 1);
    sphere.Init();

	AABox box(-2, -2, -2, 0, 0, 0);
	box.Init();

	Vector3 vertices[4];
	vertices[0] = Vector3( -1, 3,  0);
    vertices[1] = Vector3( -3, 3, 0);
    vertices[2] = Vector3(-3, 1, 0);    
	vertices[3] = Vector3( -1, 1, 0);
	Plane plane(vertices);
	plane.Init();

	InfinitePlane infinPlane(Vector3(0, 0, -10), Vector3(0, 0, 1));
	infinPlane.Init();

    Vector3 light(0, 0, 10);

	LinearAccel accel;
	accel.AddGeometry(&sphere);
	accel.AddGeometry(&box);
	accel.AddGeometry(&plane);
	accel.AddGeometry(&infinPlane);
	accel.Init();

	Ray				ray;
	Ray             shadow;
    Intersection	intersection;
	SurfaceElement	surfel;
    
    ScanLineBucket scanLineBucket(640, 480);    
    PrimarySampler primarySampler;

    primarySampler.SetRenderBucket(&scanLineBucket);
    primarySampler.Reset(0, 0, 480, 640);

    Apollo::PrimarySample sample(&image);

    while (primarySampler.GetNextSample(sample)) {
        camera.GenerateRay(ray, sample.xFilm, sample.yFilm);
        intersection.Reset();
		if (accel.Intersect(ray, intersection)) {
            surfel.Init(&intersection, &ray);

			Vector3 lightVec(light - surfel.iPoint);
			shadow.Set(surfel.iPoint, lightVec);
			DOUBLE ln = shadow.direction.Dot(surfel.normal);
			if (ln > 0) {
                FLOAT lnf = (FLOAT)ln;
				if (intersection.primitive->GetParentModel() == &sphere) {
                    sample.SetColor(Color4f(lnf, 0, 0));				    
                } else if (intersection.primitive->GetParentModel() == &box) {
					sample.SetColor(Color4f(lnf, lnf, 0));
				} else if (intersection.primitive->GetParentModel() == &plane) {
                    sample.SetColor(Color4f(0, 0, lnf));
                } else {
					sample.SetColor(Color4f(lnf, lnf, lnf));
				}
            } else {
				sample.SetColor(Color4f(1, 0, 1));
			}
		} else {
            sample.SetColor(Apollo::Color4f::ZERO());
        }
    }    

    m_timer.Stop();

    bool passed = true;
    passed &= ApolloTestFramework::Instance()->ProcessResult(this, &image, "BasicIntersection.png");
    return passed;
}

void BasicIntersectionTest::Cleanup() {
    Apollo::ImageIO::Destroy();
}
