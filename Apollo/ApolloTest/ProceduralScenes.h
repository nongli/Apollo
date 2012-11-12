/**
 * Static utility class for generating procedural scenes
 *
 */
#pragma once

#include <vector>
#include "ApolloEngine.h"

class ProceduralScenes {
public:
	static Apollo::Camera* KochSnowFlake(std::vector<Apollo::Model*>& model, std::vector<Apollo::Light*>& lights, int depth, Apollo::Sphere* sphere = NULL);
    static Apollo::Camera* CornellBox(std::vector<Apollo::Model*>& model, std::vector<Apollo::Light*>& lights);

private:
	static void GenerateSnowFlake(const std::vector<Apollo::Shader*>& shaders, std::vector<Apollo::Model*>& model, Apollo::Sphere* sphere, int depth);
};