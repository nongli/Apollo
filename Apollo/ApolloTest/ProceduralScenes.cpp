#include "ProceduralScenes.h"

using namespace std;
using namespace Apollo;

void ProceduralScenes::GenerateSnowFlake(const vector<Shader*>& shaders, vector<Model*>& models, Sphere* sphere, int depth) {
	int shader_idx = depth % shaders.size();
	sphere->SetShader(shaders[shader_idx]);
	models.push_back(sphere);
	if (depth == 0) return;

	DOUBLE locoffset	= sphere->radius / 2 + sphere->radius;
	FLOAT size			= sphere->radius / 2;
	Vector3 loc	= sphere->center;

	GenerateSnowFlake(shaders, models, new Sphere(Vector3(loc.x, loc.y + locoffset, loc.z), size), depth - 1);
	GenerateSnowFlake(shaders, models, new Sphere(Vector3(loc.x, -(loc.y + locoffset), loc.z), size), depth - 1);

	GenerateSnowFlake(shaders, models, new Sphere(Vector3(loc.x + locoffset, loc.y, loc.z), size), depth - 1);
	GenerateSnowFlake(shaders, models, new Sphere(Vector3(-(loc.x + locoffset), loc.y, loc.z), size), depth - 1);

	GenerateSnowFlake(shaders, models, new Sphere(Vector3(loc.x, loc.y, loc.z + locoffset), size), depth - 1);
	GenerateSnowFlake(shaders, models, new Sphere(Vector3(loc.x, loc.y, -(loc.z + locoffset)), size), depth - 1);
}

Camera* ProceduralScenes::KochSnowFlake(vector<Model*>& models, vector<Light*>& lights, int depth, Sphere* sphere) {
	if (sphere == NULL) {
		sphere = new Sphere();
	}
	vector<Shader*> shaders;
	shaders.push_back(new LambertianShader(Color4f::RED()));
	shaders.push_back(new LambertianShader(Color4f::GREEN()));
	shaders.push_back(new LambertianShader(Color4f::BLUE()));

	lights.push_back(Light::CreatePointLight(Vector3(0, 0, 10), Color4f(1, 1, 1), 1));
	GenerateSnowFlake(shaders, models, sphere, depth);
	return new PerspectiveCamera(Vector3(5, 0, 3), Vector3(-5, 0, -3));
}
