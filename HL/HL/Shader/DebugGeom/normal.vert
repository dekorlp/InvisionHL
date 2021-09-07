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


layout(location = 0) out vec3 normal;

void main() {
	mat3 normalMatrix = mat3(transpose(inverse(genUbo.view * ubo.model)));
    normal =  vec3(vec4(normalMatrix * inNormal, 0.0));
	gl_Position = genUbo.view * ubo.model * vec4(inPosition, 1.0);
}