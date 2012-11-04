#include "RenderTests.h"

using namespace Apollo;
using namespace std;

bool CheckerboardTest::Execute() {    
	auto_ptr<Image> checkerboard(ApolloTestFramework::LoadImage("Checkerboard.png", ApolloTestFramework::DATA_IMAGE));
    Texture2D texture(checkerboard.get());
    TextureSampler<Color4f> textureSampler(&texture);

    Image image(640, 480);

    m_timer.Start();
    PerspectiveCamera camera(Vector3(1, .6, -5), Vector3(-1, -.6, 5), Vector3(0, 1, 0));
	camera.SetImageDimension(640, 480);
	camera.SetAspectRatio(((FLOAT)640)/480);	

	Vector3 vertices[4];
	DOUBLE size = 20;
	vertices[0] = Vector3(-size, 0,  size);
	vertices[1] = Vector3( size, 0,  size);
    vertices[2] = Vector3( size, 0, -size);
    vertices[3] = Vector3(-size, 0, -size);
	Plane plane(vertices);
	plane.Init();

    Sphere sphere(Vector3(0, 1.1, 0), .5f);
    sphere.Init();

    Vector3 light(0, 20, 0);

	LinearAccel accel;
	accel.AddGeometry(&plane);
    //accel.AddGeometry(&sphere);
	accel.Init();

	Ray				ray;
	Ray             shadow;
    Intersection	intersection;
	SurfaceElement	surfel;
    
    ScanLineBucket scanLineBucket(640, 480);    
    PrimarySampler primarySampler;
	//GridSuperSampler primarySampler(1);
	//primarySampler.SetPixelSize(1.15f);

    primarySampler.SetRenderBucket(&scanLineBucket);
    primarySampler.Reset(0, 0, 480, 640);

    Apollo::PrimarySample sample(&image);

    while (primarySampler.GetNextSample(sample)) {
        camera.GenerateRay(ray, sample.xFilm, sample.yFilm);
        intersection.Reset();

        Color4f color;
		if (accel.Intersect(ray, intersection)) {
            surfel.Init(&intersection, &ray);

            if (surfel.intersection->primitive->GetParentModel() == &plane) {                
			    Vector3 lightVec(light - surfel.iPoint);
			    shadow.Set(surfel.iPoint, lightVec);

			    DOUBLE ln = shadow.direction.Dot(surfel.normal);
			    if (ln > 0) {
				    Color4f c;
				    FLOAT u = surfel.uv.u;
				    FLOAT v = surfel.uv.v;
				    u = u * 4;
				    v = v * 4;
				    while (u > 1) u -= 1;
				    while (v > 1) v -= 1;
				    UV uv(u, v);
				    textureSampler.GetSample(uv, c);
				    c *= (FLOAT)ln;
                    color = c;
			    }
            }
            else if (surfel.intersection->primitive->GetParentModel() == &sphere) {         
                Vector3 reflect = ray.direction.ReflectVector(surfel.normal);
                shadow.Set(surfel.iPoint, reflect);
                intersection.Reset();
                if (accel.Intersect(shadow, intersection)) {
                    surfel.Init(&intersection, &shadow);
                    if (surfel.intersection->primitive->GetParentModel() == &plane) {                                                
			            Color4f c;
			            FLOAT u = surfel.uv.u;
			            FLOAT v = surfel.uv.v;
		                u = u * 4;
			            v = v * 4;
			            while (u > 1) u -= 1;
			            while (v > 1) v -= 1;
			            UV uv(u, v);
			            textureSampler.GetSample(uv, c);
                        color = c;
                    } else {
                        color = Color4f(1, 0, 1);
                    }
                } else {
                    color = Color4f(0, .8f, .8f);
                }
            }
            color.a = 1;
		} else {
            color = Color4f(0, 1, 1);
        }
        sample.SetColor(color);
    }    

    m_timer.Stop();

    bool passed = true;
    passed &= ApolloTestFramework::Instance()->ProcessResult(this, &image, "CheckerboardTest.png");
    return passed;
}

