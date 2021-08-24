#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

struct Material
{
	float specularStrength;
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

layout(location = 0) in vec3 normal[];




const float MAGNITUDE = 0.5;

void GenerateLine(int index)
{
    gl_Position = genUbo.proj * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = genUbo.proj * (gl_in[index].gl_Position + 
                                vec4(normal[index], 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main() {
   GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal
}