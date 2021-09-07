#version 450
#extension GL_ARB_separate_shader_objects : enable

struct Material
{
	float shininess;
};

layout(binding = 0) uniform UniformBufferObject {
	Material material;
    mat4 model;
} ubo;

layout(binding = 1) uniform GeneralUniformBufferObject {
	mat4 view;
	mat4 proj;
	vec3 viewPos;
} genUbo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec3 fragNormal;

void main() {
	vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
	fragPos = worldPos.xyz;
	fragColor = inColor;
	//fragTexCoord = inTexCoord;
	fragNormal = vec3(transpose(inverse(ubo.model))* vec4(inNormal, 1.0)).xyz;

	gl_Position = genUbo.proj * genUbo.view * worldPos;
}