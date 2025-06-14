#version 450

layout (location = 0) in vec3 inPos;
layout (location = 0) out vec4 outColor;
layout (binding = 2) uniform samplerCube samplerEnv;

layout(push_constant) uniform PushConstants{
    mat4 mvp;
    bool first;
    float deltaPhi;
	float deltaTheta;
	float roughness;
	uint numSamples;
} consts;

#define PI 3.1415926535897932384626433832795

void main()
{
	vec3 N = normalize(inPos);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up = cross(N, right);

	const float TWO_PI = PI * 2.0;
	const float HALF_PI = PI * 0.5;

	vec3 color = vec3(0.0);
	uint sampleCount = 0u;
	for (float phi = 0.0; phi < TWO_PI; phi += consts.deltaPhi) {
		for (float theta = 0.0; theta < HALF_PI; theta += consts.deltaTheta) {
			vec3 tempVec = cos(phi) * right + sin(phi) * up;
			vec3 sampleVector = cos(theta) * N + sin(theta) * tempVec;
			color += texture(samplerEnv, sampleVector).rgb * cos(theta) * sin(theta);
			//color *= 0.3f;
			sampleCount++;
		}
	}
	outColor = vec4(PI * color / float(sampleCount), 1.0);
}
