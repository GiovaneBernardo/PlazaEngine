#version 450
layout(location = 0) in vec3 fragTexCoord;
//layout (location = 0) out vec4 gPosition;
layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gDiffuse;
layout (location = 2) out vec4 gOthers;

layout(binding = 7) uniform samplerCube prefilterMap;

layout(push_constant) uniform PushConstants{
    mat4 projection;
    mat4 view;
	float skyboxIntensity;
	float gamma;
	float exposure;
    float useless;
} pushConstants;

void main() {
    //vec3 color = vec3(pow(texture(irradianceMap, fragTexCoord).xyz, vec3(1.0f / pushConstants.gamma)));
	vec3 color =  texture(prefilterMap, fragTexCoord.xyz).xyz;
	color = color * pushConstants.skyboxIntensity;

    gDiffuse = vec4(color, 1.0f);
}
