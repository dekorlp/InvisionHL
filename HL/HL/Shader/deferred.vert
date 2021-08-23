#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 1) out vec2 textureCord;

void main() {
	textureCord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(textureCord * 2.0f - 1.0f, 0.0f, 1.0f);
}