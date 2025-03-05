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

vec3 Uncharted2Tonemap(vec3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec3 inverseTonemapApprox(vec3 color) {
    float exposure = pushConstants.exposure;
    return pow(color, vec3(1.0 / 0.5)) / 2;
}

void main() {
    //vec3 color = vec3(pow(texture(irradianceMap, fragTexCoord).xyz, vec3(1.0f / pushConstants.gamma)));
	vec3 color =  texture(prefilterMap, fragTexCoord.xyz).xyz;
	color = (color);

    gDiffuse = vec4(color, 1.0f);
}
