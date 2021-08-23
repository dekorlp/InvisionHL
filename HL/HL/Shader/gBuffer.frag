#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;


void main() {
	outPosition = vec4(fragPos, 1.0);
	outNormal = vec4(fragNormal, 1.0);
	outAlbedo = vec4(fragColor, 1.0); //texture(texSampler, fragTexCoord);
}