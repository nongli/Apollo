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
	
    m_timer.Resume();
    scene.Initialize();
    WhittedRayTracer::Settings settings;
    WhittedRayTracer tracer(settings);
    const Image* image = tracer.RenderAll(&scene);
    
    PathTracer::Settings path_tracer_settings;
    path_tracer_settings.samples_per_pixel = 5;
    path_tracer_settings.max_depth = 2;
    PathTracer path_tracer(path_tracer_settings);
    const Image* path_image = path_tracer.RenderAll(&scene);
    ApolloTestFramework::Instance()->SaveImage(this, path_image, "CornellBoxPath.png", ApolloTestFramework::RESULT_MISC);
    m_timer.Pause();

    bool passed = true;
    passed &= ApolloTestFramework::Instance()->ProcessResult(this, image, "CornellBoxRay.png");
    return passed;
}

