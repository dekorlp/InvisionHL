#version 450
#extension GL_ARB_separate_shader_objects : enable

struct Material
{
	float diffuse;
	float specular;
	float shininess;
	float shininess1;
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
layout(location = 2) out vec3 normal;

void main() {
    gl_Position = genUbo.proj * genUbo.view * ubo.model * vec4(inPosition, 1.0);
	fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
	normal = mat3(transpose(inverse(ubo.model))) * inNormal;
    fragColor = inColor;
}