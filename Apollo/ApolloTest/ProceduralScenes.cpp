#include "ProceduralScenes.h"

using namespace std;
using namespace Apollo;

void AddModel(vector<Model*>& models, Model* model, Shader* shader) {
    model->SetShader(shader);
    models.push_back(model);
}

Camera* ProceduralScenes::CornellBox(vector<Model*>& models, vector<Light*>& lights) {
    lights.push_back(Light::CreatePointLight(Vector3(0, 9.9f, -5), Color4f::WHITE(), 1));

    LambertianShader* floor_shader = new LambertianShader(Color4f::FromRGB(230, 190, 120));
    LambertianShader* left_wall_shader = new LambertianShader(Color4f::FromRGB(180, 35, 20));
    LambertianShader* right_wall_shader = new LambertianShader(Color4f::FromRGB(40, 92, 19));

    Vector3 floor[4];
	floor[0] = Vector3(-6, 0, -10);
	floor[1] = Vector3(-6, 0, 0);
    floor[2] = Vector3(6, 0, 0);    
    floor[3] = Vector3(6, 0, -10);

    Vector3 ceiling[4];
    ceiling[0] = Vector3(-6, 10, -10);
    ceiling[1] = Vector3(6, 10, -10);
    ceiling[2] = Vector3(6, 10, 0);    
	ceiling[3] = Vector3(-6, 10, 0);

    Vector3 back_wall[4];
    back_wall[0] = Vector3(6, 10, -10);
    back_wall[1] = Vector3(-6, 10, -10);
    back_wall[2] = Vector3(-6, 0, -10);    
	back_wall[3] = Vector3(6, 0, -10);

    Vector3 left_wall[4];
    left_wall[0] = Vector3(6, 10, -10);
	left_wall[1] = Vector3(6, 0, -10);
    left_wall[2] = Vector3(6, 0, 0);    
    left_wall[3] = Vector3(6, 10, 0);

    Vector3 right_wall[4];
    right_wall[0] = Vector3(-6, 10, -10);
    right_wall[1] = Vector3(-6, 10, 0);
    right_wall[2] = Vector3(-6, 0, 0);    
	right_wall[3] = Vector3(-6, 0, -10);

    AddModel(models, new Plane(floor), floor_shader);
    AddModel(models, new Plane(ceiling), floor_shader);
    AddModel(models, new Plane(back_wall), floor_shader);
    AddModel(models, new Plane(left_wall), left_wall_shader);
    AddModel(models, new Plane(right_wall), right_wall_shader);

	return new PerspectiveCamera(Vector3(0, 5, 12), Vector3(0, 0, -1));
}

void ProceduralScenes::GenerateSnowFlake(const vector<Shader*>& shaders, vector<Model*>& models, Sphere* sphere, int depth) {
	int shader_idx = depth % shaders.size();
    AddModel(models, sphere, shaders[shader_idx]);
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
