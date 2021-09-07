#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D texAlbedo;
layout(binding = 1) uniform sampler2D texNormal;
layout(binding = 2) uniform sampler2D texPosition;
layout(binding = 3) uniform sampler2D shadowMap;

layout(location = 1) in vec2 textureCord;

layout(location = 0) out vec4 outColor;

layout(binding = 4) uniform OptionsUniformBuffer {
	int option;
} uob;

struct Light
{
	mat4 lightSpaceMatrix;
	vec4 position;
	vec4 color;
	float strength;
	float direction;
	float falloffstart;
	float falloffEnd;
	float spotPower;
};

layout(set = 0, binding = 5) uniform LightUbo {
	Light lights[8];
	int countLights;
} lUbo;

layout(set = 0, binding = 6) uniform GeneralUniformBufferObject {
	mat4 view;
	mat4 proj;
	vec3 viewPos;
} genUbo;

const int NUM_LIGHTS = 8;

float LinearizeDepth(float depth)
{
  float n = 1.0; // camera z near
  float f = 128.0; // camera z far
  float z = depth;
  return (2.0 * n) / (f + n - z * (f - n));	
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 FragPos, vec3 Normal, int lightIndex)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
	float lsvdepth = texture(shadowMap, projCoords.xy).r;
	float vdepth =  fragPosLightSpace.z / lUbo.lights[lightIndex].position.z;
	
	if(vdepth <= lsvdepth)
		return 0;
	
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightShadowDir = normalize(lUbo.lights[lightIndex].position.xyz - FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightShadowDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1)
        shadow = 0.0;
        
    return shadow;
}

void main() {
	switch(uob.option)
	{
		case 1:
		{
			outColor = texture(texAlbedo, textureCord);
			break;
		}
		case 2:
		{		
			outColor = texture(texNormal, textureCord);
			break;
		}
		case 3:
		{
			outColor = texture(texPosition, textureCord);
			break;
		}
		case 4: // lightning
		{
			// src: https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
			
			
			
			
			vec3 FragPos = texture(texPosition, textureCord).rgb;
			vec3 color = texture(texAlbedo, textureCord).rgb;
			vec3 normal = texture(texNormal, textureCord).rgb;
			
			
			
			vec3 viewDir = normalize(genUbo.viewPos - FragPos);
			
			// ambient
			float ambientStrength = 0.1f;
			vec3 ambient = ambientStrength * lUbo.lights[0].color.xyz;
			vec3 result = ambient;
			
			for(int i = 0; i < NUM_LIGHTS; ++i)
			{
				

				// diffuse
				vec3 norm = normalize(normal);
				vec3 lightDir = normalize(lUbo.lights[i].position.xyz - FragPos);
				float diff = max(dot(norm, lightDir), 0.0f);
				vec3 diffuse = diff * color * lUbo.lights[i].color.xyz;
		
				// specular
				float specularStrength = 0.5;
				
				vec3 halfayDir = normalize(lightDir + viewDir);
				float spec = pow(max(dot(norm, halfayDir), 0.0), 16);
				vec3 specular = specularStrength * spec * lUbo.lights[i].color.xyz;  
				
				result += diffuse + specular;
				
			
			}
			
			
			outColor = vec4(result, 1.0);
			
			
			break;
		}
		case 5: // lightning and shadows
		{
			// src: https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
		
			vec3 FragPos = texture(texPosition, textureCord).rgb;
			vec3 color = texture(texAlbedo, textureCord).rgb;
			vec3 normal = texture(texNormal, textureCord).rgb;
			
			
			
			vec3 viewDir = normalize(genUbo.viewPos - FragPos);
			
			// ambient
			float ambientStrength = 0.1f;
			vec3 ambient = ambientStrength * lUbo.lights[0].color.xyz;
			vec3 result = ambient;
			
			for(int i = 0; i < NUM_LIGHTS; ++i)
			{
				

				// diffuse
				vec3 norm = normalize(normal);
				vec3 lightDir = normalize(lUbo.lights[i].position.xyz - FragPos);
				float diff = max(dot(norm, lightDir), 0.0f);
				vec3 diffuse = diff * color * lUbo.lights[i].color.xyz;
		
				// specular
				float specularStrength = 0.5;
				
				vec3 halfayDir = normalize(lightDir + viewDir);
				float spec = pow(max(dot(norm, halfayDir), 0.0), 16);
				vec3 specular = specularStrength * spec * lUbo.lights[i].color.xyz;  
				
				float shadow = ShadowCalculation( lUbo.lights[i].lightSpaceMatrix  * vec4(FragPos, 1.0), FragPos, normal, i);
				result += (diffuse + specular) *(1.0 - 0.75 * shadow) ;
				
			
			}
			
			
			outColor = vec4(result, 1.0);
			break;
		
		
		
		
			
			//vec3 FragPos = texture(texPosition, textureCord).rgb;
		
			//vec3 color = texture(texAlbedo, textureCord).rgb;
			//vec3 normal = texture(texNormal, textureCord).rgb;
			
			// ambient
			//float ambientStrength = 0.1f;
			//vec3 ambient = ambientStrength * lUbo.lights[0].color.xyz;

			// diffuse
			//vec3 norm = normalize(normal);
			//vec3 lightDir = normalize(lUbo.lights[0].position.xyz - FragPos);
			//float diff = max(dot(norm, lightDir), 0.0f);
			//vec3 diffuse = diff * lUbo.lights[0].color.xyz;
	
			// specular
			//float specularStrength = 0.5;
			//vec3 viewDir = normalize(genUbo.viewPos - FragPos);
			//vec3 reflectDir = reflect(-lightDir, norm);
			//float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
			//vec3 specular = specularStrength * spec * lUbo.lights[0].color.xyz;  
	
			// calculate light and shadow
			//float shadow = ShadowCalculation( lUbo.lights[0].lightSpaceMatrix  * vec4(FragPos, 1.0), FragPos, normal);
			//vec3 lightning = (ambient + (1.0 - 0.99 * shadow) * (diffuse + specular)) * color; 
	
			//outColor = vec4((ambient + diffuse + specular)  * color, 1.0);
			//outColor = vec4(lightning, 1.0);
		
			//break;
		}
			
		case 6: // debug shadow Map
		{
			float depthValue = texture(shadowMap, textureCord).r;
			outColor = vec4(vec3(1.0-LinearizeDepth(depthValue)), 1.0);
			break;
		}
	}
	

   
}