#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec3 normal;

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

layout(set = 0, binding = 1) uniform GeneralUniformBufferObject {
	mat4 view;
	mat4 proj;
	vec3 viewPos;
} genUbo;

layout(set = 0, binding = 2) uniform LightUbo {
	Light lights[8];
	int countLights;
} lUbo;

layout(location = 0) out vec4 outColor;

void main() {
	// ambient
	float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * lUbo.lights[0].color.xyz;

	// diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lUbo.lights[0].position.xyz - fragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * lUbo.lights[0].color.xyz;
	
	// specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(genUbo.viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lUbo.lights[0].color.xyz;  
	
	vec3 result = (ambient + diffuse + specular) * fragColor;
	
    outColor = vec4(result, 1.0);
}