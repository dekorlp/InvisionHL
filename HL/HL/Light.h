#pragma once

#include "Engine/InCommon.h"
#include "Engine/renderer/GraphicsFactory.h"

#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum LightIndex {LIGHT_INDEX_ONE, LIGHT_INDEX_TWO, LIGHT_INDEX_THREE, LIGHT_INDEX_FOUR, LIGHT_INDEX_FIVE, LIGHT_INDEX_SIX, LIGHT_INDEX_SEVEN, LIGHT_INDEX_EIGHT};

struct SLight
{
	
	__declspec(align(16)) glm::vec4 position;
	__declspec(align(16)) glm::vec4 color;
	__declspec(align(16)) float strength;
	//__declspec(align(16)) float direction;
	//__declspec(align(16)) float falloffStart;
	//__declspec(align(16)) float falloffEnd;
	//__declspec(align(16)) float spotPower;
};

class Light
{
public:
	Light();
	Light(glm::vec3 &position, glm::vec3 &color);
	void SetStrength(float strength);
	void SetPosition(glm::vec3 &position);
	void SetColor(glm::vec3 &color);
	glm::vec3& GetColor();
	glm::vec3& GetPosition();
	SLight GetLightInformations();
	void SetLightInformations(SLight& light);
private:
		SLight mLight;
};