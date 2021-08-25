#version 450
#extension GL_ARB_separate_shader_objects : enable



struct Light
{
	vec4 position;
	vec4 color;
	float strength;
	
	//float direction;
	//float falloffstart;
	//float falloffEnd;
	//float spotPower;
};

struct Material
{
	float specularStrength;
	float shininess;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
	Material material;
    mat4 model;
} ubo;

layout(set = 0, binding = 1) uniform LightUbo {
	Light lights[8];
	int countLights;
	mat4 lightSpaceMatrix;
} lUbo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

out gl_PerVertex 
{
    vec4 gl_Position;   
};


void main() {
	gl_Position =  lUbo.lightSpaceMatrix * ubo.model  * vec4(inPosition, 1.0);
}