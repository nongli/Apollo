#include "AccelTest.h"
#include "ProceduralScenes.h"

using namespace Apollo;
using namespace std;

bool AccelTest::Execute() {
    Scene scene;

	// Kock Snow Flake
	vector<Model*> models;
	vector<Light*> lights;
	Camera* camera = ProceduralScenes::KochSnowFlake(models, lights, 3);
	for (size_t i = 0; i < lights.size(); ++i) {
		lights[i]->SetCastShadow(false);
	}

	camera->SetImageDimension(640, 480);
	camera->SetAspectRatio(((FLOAT)640)/480);

	scene.SetCamera(camera);
	scene.AddModels(models);
	scene.AddLights(lights);
	
    m_timer.Start();
    scene.Initialize();
    WhittedRayTracer tracer;
    const Image* image = tracer.RenderAll(&scene);
    m_timer.Stop();

    bool passed = true;
    passed &= ApolloTestFramework::Instance()->ProcessResult(this, image, "Snowflake.png");
    return passed;
}