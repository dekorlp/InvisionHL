#include "Light.h"

Light::Light()
{
	mLight.position = glm::vec4(0.0f);
	mLight.color = glm::vec4(0.0f);
	mLight.direction = 4.0;
}

Light::Light(glm::vec3 &position, glm::vec3 &color)
{
	mLight.position = glm::vec4(position.x, position.y, position.z, 0.0f);
	mLight.color = glm::vec4(color.x, color.y, color.z, 0.0f);
	mLight.strength = 2.0f;
	mLight.direction = 4.0;
	mLight.falloffStart = 12.0f;
	mLight.falloffEnd = 13.0f;
	mLight.spotPower = 14.0f;
}

void Light::SetStrength(float strength)
{
	mLight.strength = strength;
}

void Light::SetPosition(glm::vec3 &position)
{
	mLight.position = glm::vec4(position.x, position.y, position.z, 0.0f);
}

void Light::SetColor(glm::vec3 &color)
{
	mLight.color = glm::vec4(color.x, color.y, color.z, 0.0f);
}

glm::vec3& Light::GetColor()
{
	return glm::vec3(mLight.color.x, mLight.color.y, mLight.color.z);
}

glm::vec3& Light::GetPosition()
{
	return glm::vec3(mLight.position.x, mLight.position.y, mLight.position.z);
}

SLight Light::GetLightInformations()
{
	return mLight;
}

void Light::SetLightInformations(SLight& light)
{
	mLight = light;
}