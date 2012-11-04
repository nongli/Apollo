#include "CameraTests.h"

using namespace Apollo;

bool PerspectiveCameraBasicTest::Execute() {    
    Image image(640, 480);

    m_timer.Start();
    PerspectiveCamera camera(Vector3(0, 10, 0), Vector3(0, -1, 0), Vector3(0, 0, 1));
	camera.SetImageDimension(640, 480);
	camera.SetAspectRatio(((FLOAT)640)/480);
	
    Sphere sphere1(Vector3(-2, 0, 2), 1);
	Sphere sphere2(Vector3(2, 0, 2), 1);
	Sphere sphere3(Vector3(-2, 0, -2), 1);
	Sphere sphere4(Vector3(2, 0, -2), 1);

	sphere1.Init();
	sphere2.Init();
	sphere3.Init();
	sphere4.Init();

	Vector3 light(0, 10, 0);

	LinearAccel accel;
	accel.AddGeometry(&sphere1);
	accel.AddGeometry(&sphere2);
	accel.AddGeometry(&sphere3);
	accel.AddGeometry(&sphere4);
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
                
                if (intersection.primitive->GetParentModel() == &sphere1) {
                    sample.SetColor(Color4f(lnf, 0, 0));					    
                } else if (intersection.primitive->GetParentModel() == &sphere2) {
					sample.SetColor(Color4f(0, lnf, 0));
				} else if (intersection.primitive->GetParentModel() == &sphere3) {
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
    passed &= ApolloTestFramework::Instance()->ProcessResult(this, &image, "PerspectiveCameraBasicTest.png");
    return passed;
}

