#include "Light.h"

using namespace Apollo;

Light* Light::CreatePointLight(const Vector3& pos, const Color4f& color, FLOAT intensity) {
	Light* light = new Light(Light::POINT);
	light->SetPosition(pos);
	light->SetColor(color);
	light->SetIntensity(intensity);
	light->SetCastShadow(true);
	return light;
}

Light* Light::CreateAmbientLight(const Color4f& color, FLOAT intensity) {
	Light* light = new Light(Light::AMBIENT);
	light->SetColor(color);
	light->SetIntensity(intensity);
	return light;
}