#include "RenderTests.h"
#include "ProceduralScenes.h"

using namespace Apollo;
using namespace std;

bool CornellBoxTest::Execute() {    
    Scene scene;

	vector<Model*> models;
	vector<Light*> lights;
	Camera* camera = ProceduralScenes::CornellBox(models, lights);

	camera->SetImageDimension(640, 480);
	camera->SetAspectRatio(((FLOAT)640)/480);

	scene.SetCamera(camera);
	scene.AddModels(models);
	scene.AddLights(lights);
	
    m_timer.Start();
    scene.Initialize();
    WhittedRayTracer::Settings settings;
    WhittedRayTracer tracer(settings);
    const Image* image = tracer.RenderAll(&scene);
    m_timer.Stop();

    bool passed = true;
    passed &= ApolloTestFramework::Instance()->ProcessResult(this, image, "CornellBoxRay.png");
    return passed;
}

