#version 450

#define LIGHT_COUNT 1

layout (triangles, invocations = LIGHT_COUNT) in;
layout (triangle_strip, max_vertices = 3) out;

struct Light
{
	vec4 position;
	vec4 color;
	float strength;
	mat4 lightSpaceMatrix;
	float direction;
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
} lUbo;

layout (location = 0) in int inInstanceIndex[];

void main() 
{
	vec4 instancedPos = vec4(0.0, 0.0, 0.0, 1.0); //ubo.instancePos[inInstanceIndex[0]]; 
	for (int i = 0; i < gl_in.length(); i++)
	{
		gl_Layer = gl_InvocationID;
		vec4 tmpPos = gl_in[i].gl_Position + instancedPos;
		gl_Position = lUbo.lights[0].lightSpaceMatrix * tmpPos;
		EmitVertex();
	}
	EndPrimitive();
}
