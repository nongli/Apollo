#include "RenderTests.h"

using namespace Apollo;

bool BasicTextureTest::Execute() {    
    std::auto_ptr<Image> lena(ApolloTestFramework::LoadImage("Lena.png", ApolloTestFramework::DATA_IMAGE));
    Texture2D texture(lena.get());
    TextureSampler<Color4f> textureSampler(&texture);

    Image image(640, 480);

    m_timer.Start();
    PerspectiveCamera camera(Vector3(0, 10, 0), Vector3(0, -1, 0), Vector3(0, 0, 1));
	camera.SetImageDimension(640, 480);
	camera.SetAspectRatio(((FLOAT)640)/480);
	
    Sphere sphere(Apollo::Vector3(3, 0, 3), 1);
    sphere.Init();

	AABox box(-2, -2, -2, 0, 0, 0);
	box.Init();

	Vector3 vertices[4];
	vertices[0] = Vector3(-3, 0,  3);
	vertices[1] = Vector3( 3, 0,  3);
    vertices[2] = Vector3( 3, 0, -3);
    vertices[3] = Vector3(-3, 0, -3);  
	Plane plane(vertices);
	plane.Init();

	InfinitePlane infinPlane(Vector3(0, 0, -10), Vector3(0, 0, 1));
	infinPlane.Init();

    Vector3 light(0, 10, 0);

	LinearAccel accel;
	accel.AddGeometry(&sphere);
	accel.AddGeometry(&plane);
	//accel.AddGeometry(&infinPlane);
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

        Color4f color;

		if (accel.Intersect(ray, intersection)) {
            surfel.Init(&intersection, &ray);

			Vector3 lightVec(light - surfel.iPoint);
			shadow.Set(surfel.iPoint, lightVec);

			DOUBLE ln = shadow.direction.Dot(surfel.normal);
			if (ln > 0) {
                FLOAT lnf = (FLOAT)ln;
                Color4f c;
                textureSampler.GetSample(surfel.uv, c);
                color = c * lnf;
                /*
                if (intersection.primitive->GetParentModel() == &sphere)
                {
                    sample.SetColor(Color4f(lnf, 0, 0));					    
                }
				else if (intersection.primitive->GetParentModel() == &box)
				{
					sample.SetColor(Color4f(lnf, lnf, 0));
				}
				else if (intersection.primitive->GetParentModel() == &plane)
                {
                    sample.SetColor(Color4f(0, 0, lnf));
                }
				else
				{
					sample.SetColor(Color4f(lnf, lnf, lnf));
				}
                */
            } else {
                color.Set(1, 0, 1);
			}
            // If I hit something, alpha is always 1.
            color.a = 1;
		}
        else {
            color = Color4f::ZERO();
        }
        
        sample.SetColor(color);
    }    

    m_timer.Stop();

    bool passed = true;
    passed &= ApolloTestFramework::Instance()->ProcessResult(this, &image, "BasicTextureTest.png");
    return passed;
}

