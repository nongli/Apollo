#include "RenderTests.h"

using namespace Apollo;

BasicSceneTest::BasicSceneTest() : ApolloTest() {
}

void BasicSceneTest::Init() {
    ImageIO::Init();
    Timer::Init();
    SamplerUtil::Init(false);
}

bool BasicSceneTest::Execute() {    
    m_timer.Start();
    
    PerspectiveCamera camera;
	camera.SetImageDimension(640, 480);
	camera.SetAspectRatio(((FLOAT)640)/480);
	camera.SetPosition(Apollo::Vector3(0, 0, 10));	
	
    Sphere sphere(Apollo::Vector3(2, 1, 0), 1);
    Sphere sphere2(Apollo::Vector3(2.5, -2, 0), 1);
	AABox box(-2, -2, -2, 0, 0, 0);
	InfinitePlane infinPlane(Vector3(0, 0, -10), Vector3(0, 0, 1));
	Vector3 vertices[4];
	vertices[0] = Vector3( -1, 3,  0);
    vertices[1] = Vector3( -3, 3, 0);
    vertices[2] = Vector3(-3, 1, 0);    
	vertices[3] = Vector3( -1, 1, 0);
	Plane plane(vertices);
    
    LambertianShader redShader(Color4f(1, 0, 0));
    LambertianShader greenShader(Color4f(0, 1, 0));
    LambertianShader blueShader(Color4f(0, 0, 1));
    LambertianShader grayShader(Color4f(.6f, .6f, .6f));

    PhongShader redPhong(Color4f(1, 0, 0), Color4f(1, 1, 1), 100);
    BlinnPhongShader redBlinn(Color4f(1, 0, 0), Color4f(1, 1, 1), 100);

    sphere.SetShader(&redPhong);
    sphere2.SetShader(&redBlinn);
    box.SetShader(&blueShader);
    plane.SetShader(&greenShader);
    infinPlane.SetShader(&grayShader);

    Light light(Light::POINT);
    light.SetPosition(Vector3(2, 2, 10));

    Scene scene;
    scene.SetCamera(&camera);
    scene.AddModel(&sphere);
    scene.AddModel(&sphere2);
    scene.AddModel(&box);
    scene.AddModel(&plane);
    scene.AddModel(&infinPlane);
    
    scene.AddLight(&light);
    scene.Initialize();

    WhittedRayTracer rayTracer;
    const Image* image = rayTracer.RenderAll(&scene);

    m_timer.Stop();

    bool passed = true;
    passed &= ApolloTestFramework::Instance()->ProcessResult(this, image, "BasicScene.png");
    return passed;
}

void BasicSceneTest::Cleanup() {
    Apollo::ImageIO::Destroy();
}
