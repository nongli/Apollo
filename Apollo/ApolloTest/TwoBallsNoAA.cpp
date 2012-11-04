#include "RenderTests.h"

using namespace Apollo;

bool TwoBallsNoAATest::Execute() {    
    Image image(640, 480);

    m_timer.Start();
    PerspectiveCamera camera;
	camera.SetImageDimension(640, 480);
	camera.SetAspectRatio(((FLOAT)640)/480);
	camera.SetPosition(Apollo::Vector3(0, 0, 10));	
	
    Sphere sphere1(Apollo::Vector3(2, 0, 0), 2.5);
    Sphere sphere2(Apollo::Vector3(-2, 0, 0), 2.5);

    sphere1.Init();
    sphere2.Init();
    
    Vector3 light(0, 10, 13);

	LinearAccel accel;
	accel.AddGeometry(&sphere1);
    accel.AddGeometry(&sphere2);
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

			if (accel.Intersect(shadow) == NO_HIT) {
                DOUBLE ln = shadow.direction.Dot(surfel.normal);
				ln = fabs(ln);
				if (ln > 0) {
                    FLOAT lnf = (FLOAT)ln;
                    if (intersection.primitive == &sphere1) {
                        sample.SetColor(Apollo::Color4f(lnf, 0, 0));					    
                    } else {
                        sample.SetColor(Apollo::Color4f(0, 0, lnf));
                    }
                }
			}
		} else {
            sample.SetColor(Apollo::Color4f::ZERO());
        }
    }    

    m_timer.Stop();

    bool passed = true;
    passed &= ApolloTestFramework::Instance()->ProcessResult(this, &image, "TwoBallsNoAA.png");
    return passed;
}
