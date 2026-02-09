#pragma extension SPV_EXT_descriptor_indexing

// Vertex shader
#include "FullscreenQuad.hlsl"
#include "Matrix.hlsl"

#define SHOW_HEATMAP 0

// Pixel shader
struct UniformBufferObject
{
    float4x4 projection;
    float4x4 view;
    bool showCascadeLevels;
    float farPlane;
    float nearPlane;
    float gamma;
    float exposure;
    int cascadeCount;
    int lightCount;
    float4 viewPos;
    float4 lightDirection;
    float4 ambientLightColor;
    float4 directionalLightColor;
    float2 screenSize;
    float3 clusterSize;
    float4x4 lightSpaceMatrices[16];
    float4 cascadePlaneDistances[16];
};


cbuffer UBO : register(b4)
{
	UniformBufferObject ubo;
}

struct LightStruct {
    float3 color;
    float radius;
    float3 position;
    float intensity;
    float cutoff;
    float minRadius;
};

struct Cluster {
	int lightsIndex[256];
	int lightsCount;
	float3 minBounds;
	float3 maxBounds;
};

struct Plane
{
    float3 Normal;
    float Distance;
};

struct Frustum
{
    Plane planes[4];
};

StructuredBuffer<LightStruct> LightsSSBO : register(t11);
StructuredBuffer<Cluster> ClustersSSBO : register(t12);

Texture2D<float4> gNormal     : register(t0);
Texture2D<float4> gDiffuse    : register(t1);
Texture2D<float4> gOthers     : register(t2);
Texture2D<float4> gSceneDepth : register(t3);

Texture2D<float4>      samplerBRDFLUT     : register(t6);
TextureCube<float4>    prefilterMap       : register(t7);
TextureCube<float4>    irradianceMap      : register(t8);
Texture2DArray<float4> shadowsDepthMap    : register(t9);
TextureCube<float4>    equirectangularMap : register(t10);
Texture2D<float4>      ssgiTexture        : register(t14);

SamplerState linearSampler : register(s13);

static const float PI = 3.14159265359f;

#define MAX_POINT_LIGHT_PER_TILE 2048

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

struct PSOutput
{
	float4 SceneColor  : SV_Target0;
};

float3 GetHeatmapColor(int lightCount, int maxLights);
float GetDepth(float2 uv);
float4x4 InverseProjection(float4x4 proj);
float3 ReconstructViewPosition(float2 uv, float depth);
float roundToMultiple(float value, float multiple);
uint GetIndex(float2 screenPos, float2 clusterCount);
float3 specularContribution(float3 L, float3 V, float3 N, float3 F0, float metallic, float roughness, float3 materialColor);
float3 CalculateDirectionalLight(float3 fragPos, float3 albedo, float3 normal, float metallic, float roughness);
float3 F_SchlickR(float cosTheta, float3 F0, float roughness);
float3 F_Schlick(float cosTheta, float3 F0);
float D_GGX(float dotNH, float roughness);
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness);
float3 CalculatePointLight(float3 fragPos, float3 albedo, float3 normal, float roughness, float3 lightPos, float3 lightColor, float metallic, float attenuation);
float CalculateAttenuation(float3 lightPos, float3 fragPos, float minRadius, float maxRadius);
float ShadowCalculation(float3 fragPos, float3 Normal);

PSOutput mainPS(PSInput input)
{
	PSOutput output;
	float2 uv = input.uv;
	float2 TexCoords = uv;
	float4 sceneColor = gDiffuse.Sample(linearSampler, uv);
	output.SceneColor = float4(sceneColor.xyz, 1.0f);

	float depth = GetDepth(uv);
    float3 lighting  = float3(1.0f, 1.0f, 1.0f);

    float2 clusterCount = ceil(ubo.screenSize / ubo.clusterSize.xy);
    uint clusterIndex = GetIndex(TexCoords, clusterCount);
	Cluster currentCluster = ClustersSSBO[clusterIndex];
    const float3 Diffuse = gDiffuse.Sample(linearSampler, uv).xyz;
    float3 color = Diffuse;
    //if(ubo.lightCount > 0 && depth != 1.0f) {
    if(depth != 1.0f) {
        const float3 fragViewPos = ReconstructViewPosition(TexCoords, depth);
        const float3 Normal = gNormal.Sample(linearSampler, uv).xyz;
        const float3 Others = gOthers.Sample(linearSampler, uv).xyz;
        const float Specular = Others.x;
        const float metalness = Others.y;
        const float roughness = Others.z;


        color = CalculateDirectionalLight(fragViewPos, Diffuse, Normal, metalness, roughness);
    	//color += fragViewPos * 1.0f;

		if (currentCluster.lightsCount > 0)
		{
//output.SceneColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
			for (int i = 0; i < currentCluster.lightsCount && currentCluster.lightsIndex[i] != -1; ++i)
			{
				LightStruct light = LightsSSBO[currentCluster.lightsIndex[i]];
                float3 lightPosition = mul(ubo.view, float4(light.position, 1.0f)).xyz;

                float3 lightColor = light.color * light.intensity;
                float attenuation =  CalculateAttenuation(lightPosition, fragViewPos, light.cutoff, light.radius + light.cutoff);
				//lighting += lightColor;
				color += (CalculatePointLight(fragViewPos, Diffuse, Normal, roughness, lightPosition, lightColor, metalness, attenuation) * 1.0f);
			}
        }
        //color = color + (lighting - 1.0f); //* (lighting);

		// Apply SSGI indirect lighting
		float3 ssgi = ssgiTexture.Sample(linearSampler, uv).rgb;
		ssgi *= float3(0.0f, 0.0f, 0.0f);
		color += ssgi * Diffuse; // Modulate by albedo for indirect diffuse
    }
    output.SceneColor = float4(color, 1.0f);

	#if SHOW_HEATMAP
    // Heatmap visualization
	float3 heatmapColor = GetHeatmapColor(currentCluster.lightsCount, ubo.lightCount);
	output.SceneColor = float4(output.SceneColor.rgb + ((heatmapColor * 0.6f) + 0.0f), 1.0f);
	#endif
	
	return output;
}

float3 GetHeatmapColor(int lightCount, int maxLights)
{
    // Normalize light count to 0-1 range
	float normalizedCount = saturate((float) lightCount / (float) max(maxLights, 1));
    
    // Create smooth heatmap: blue -> green -> yellow -> red
	float3 color;
    
	if (normalizedCount < 0.33f)
	{
        // Blue to Green (0.0 to 0.33)
		float t = normalizedCount / 0.33f; // Remap to 0-1
		color = lerp(float3(0.0f, 0.0f, 1.0f), // Blue
                     float3(0.0f, 1.0f, 0.0f), // Green
                     t);
	}
	else if (normalizedCount < 0.66f)
	{
        // Green to Yellow (0.33 to 0.66)
		float t = (normalizedCount - 0.33f) / 0.33f; // Remap to 0-1
		color = lerp(float3(0.0f, 1.0f, 0.0f), // Green
                     float3(1.0f, 1.0f, 0.0f), // Yellow
                     t);
	}
	else
	{
        // Yellow to Red (0.66 to 1.0)
		float t = (normalizedCount - 0.66f) / 0.34f; // Remap to 0-1
		color = lerp(float3(1.0f, 1.0f, 0.0f), // Yellow
                     float3(1.0f, 0.0f, 0.0f), // Red
                     t);
	}
    
    // If no lights, show dark blue
	if (lightCount == 0)
	{
		color = float3(0.0f, 0.0f, 0.2f); // Dark blue for empty clusters
	}
    
	return color;
}



float GetDepth(float2 uv) {
	return gSceneDepth.Sample(linearSampler, uv).x;
}

float4x4 InverseProjection(float4x4 proj)
{
    float4x4 inv;
    inv[0][0] = 1.0 / proj[0][0]; inv[0][1] = 0; inv[0][2] = 0; inv[0][3] = 0;
    inv[1][0] = 0; inv[1][1] = 1.0 / proj[1][1]; inv[1][2] = 0; inv[1][3] = 0;
    inv[2][0] = 0; inv[2][1] = 0; inv[2][2] = 0; inv[2][3] = 1.0 / proj[2][2]; // depends on near/far
    inv[3][0] = 0; inv[3][1] = 0; inv[3][2] = -proj[2][3] / proj[2][2]; inv[3][3] = 1;
    return inv;
}

// Depth linearization for infinite perspective
float LinearDepth(float z_buffer, float nearPlane)
{
	// z_buffer in [0,1]
	return nearPlane / (1.0 - z_buffer);
}
float3 ReconstructViewPosition(float2 uv, float depth)
{
    float4 clipPos = float4(uv * 2.0f - 1.0f, depth, 1.0f);
	clipPos.y *= -1.0f;
    float4 viewPos = mul(inverse(ubo.projection), clipPos);

    return viewPos.xyz / viewPos.w;
}

float roundToMultiple(float value, float multiple) {
    return floor(value / multiple) * multiple;
}

uint GetIndex(float2 screenPos, float2 clusterCount)
{
    float2 currentClusterPosition = float2(ceil(roundToMultiple(screenPos.x * ubo.screenSize.x, ubo.clusterSize.x) / ubo.clusterSize.x), ceil(roundToMultiple(screenPos.y * ubo.screenSize.y, ubo.clusterSize.y) / ubo.clusterSize.y));
    return uint(((currentClusterPosition.y * (clusterCount.x)) + (currentClusterPosition.x)));
}


float3 CalculateDirectionalLight(
    float3 fragPos, float3 albedo, float3 normal,
    float metallic, float roughness)
{
    float3 V = normalize(-fragPos);
    float NdotV = dot(normal, V);

    if (NdotV < 0.0f) {
        normal = -normal;
        NdotV = 1.0f - NdotV;
    }
	float3x3 viewInv3x3 = (float3x3)(inverse(ubo.view));
	float3 R = mul(viewInv3x3, reflect(-V, normal));
//
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);
//
    float maxNdotV = max(NdotV, 0.25f);
    float3 F = F_SchlickR(maxNdotV, F0, roughness);
//
    float3 kD = (1.0f - F) * (1.0f - metallic);
//
	float3x3 view3x3 = (float3x3)inverse(ubo.view);
	float3 L = normalize(mul(ubo.view, float4(ubo.lightDirection.xyz, 0.0f)).xyz);
//
    float3 Lo = specularContribution(L, V, normal, F0, metallic, roughness, albedo);
//
    float3 irradiance = irradianceMap.Sample(linearSampler, normal).rgb + ubo.ambientLightColor.x;
    float3 diffuse = irradiance * albedo.xyz;
//
    const float MAX_REFLECTION_LOD = 9.0f;
    float3 prefilteredColor = prefilterMap.SampleLevel(linearSampler, R, roughness * MAX_REFLECTION_LOD).rgb;
    float2 brdfCoord = float2(maxNdotV, roughness);
    float2 brdf = samplerBRDFLUT.Sample(linearSampler, brdfCoord).rg;
    float3 reflection = prefilterMap.SampleLevel(linearSampler, R, roughness * MAX_REFLECTION_LOD).rgb;
    float3 specular = reflection * (F * brdf.x + brdf.y);
//
    float ambientOcclusion = 1.0f;
    float3 ambient = (kD * diffuse + specular) * ambientOcclusion;

    float3 shadow = max((1.0f - ShadowCalculation(fragPos.xyz, normal)) * (ubo.directionalLightColor.xyz) + (ubo.ambientLightColor.xyz * ubo.ambientLightColor.w), 0.5f);
    float3 color = ambient + Lo;
    color *= shadow;
    return color;
}

float3 F_SchlickR(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max(float3(1.0f, 1.0f, 1.0f) - roughness, F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 F_Schlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float D_GGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2)/(PI * denom*denom);
}

float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

float3 specularContribution(float3 L, float3 V, float3 N, float3 F0, float metallic, float roughness, float3 materialColor)
{
    float3 H = normalize (V + L);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);

    float3 color = float3(0.0f, 0.0f, 0.0f);

    if (dotNL > 0.0) {
        float D = D_GGX(dotNH, roughness);
        float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
        float3 F = F_Schlick(dotNV, F0);
        float3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
        float3 kD = (float3(1.0f, 1.0f, 1.0f) - F) * (1.0 - metallic);
        color += (kD * materialColor / PI + spec) * dotNL;
    }

    return color;
}

float3 CalculatePointLight(float3 fragPos, float3 albedo, float3 normal, float roughness, float3 lightPos, float3 lightColor, float metallic, float attenuation) {
    float3 V = normalize(-fragPos);        // View direction
    float3 L = normalize(lightPos - fragPos);           // Light direction
    float NdotL = max(dot(normal, L), 0.0);           // Lambertian diffuse

    // Fresnel reflectance factor for metallic surfaces
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);      // Base reflectance
    float3 F = F_SchlickR(NdotL, F0, roughness);        // Fresnel term
    float3 kD = (1.0 - F) * (1.0 - metallic);           // Diffuse weight

    // Diffuse component
    float3 diffuse = (kD * albedo) * lightColor * NdotL;  // Basic diffuse shading
    //diffuse *= 1.0f;

    //return float3(normal);
    float3 color = diffuse + (specularContribution(L, V, normal, F0, metallic, roughness, albedo) * lightColor * 5.0f ); // or color = ambient + diffuse + specular if those terms are used
    return color * attenuation;
}

float CalculateAttenuation(float3 lightPos, float3 fragPos, float minRadius, float maxRadius) {
    float distance = length(lightPos - fragPos);

    // Compute linear attenuation based on distance and clamp within min/max range
    float attenuation = clamp(1.0 - (distance - minRadius) / (maxRadius - minRadius), 0.0, 1.0);

    // Optionally, square the attenuation to make the falloff effect smoother
    attenuation *= attenuation;

    return attenuation;
}

float3 ViewToWorldSpace(float3 viewPosition) {
	float4 worldPosition = mul(inverse(ubo.view), float4(viewPosition, 1.0));
	return worldPosition.xyz;
}

float ShadowCalculation(float3 fragPos, float3 Normal)
{
	// select cascade layer
	float4 fragPosViewSpace = float4(fragPos, 1.0f);
	float depthValue = abs(fragPosViewSpace.z);
	int layer = ubo.cascadeCount;
	for (int i = 0; i < ubo.cascadeCount - 1; ++i)
	{
		if (depthValue < ubo.cascadePlaneDistances[i].x)
		{
			layer = i;
			break;
		}
	}

	// Transform to light space
	float4 fragPosLightSpace = mul(ubo.lightSpaceMatrices[layer], float4(ViewToWorldSpace(fragPos.xyz), 1.0f));
	float4 projCoords = fragPosLightSpace / fragPosLightSpace.w;
	float currentDepth = projCoords.z;
	projCoords = projCoords * 0.5f + 0.5f;
	projCoords.y = 1.0f - projCoords.y;

	float shadowFromDepthMap = shadowsDepthMap.Sample(linearSampler, float3(projCoords.xy, (float)layer)).r;

	// Keep shadow at 0.0 when outside far plane
	if (currentDepth > 1.0f)
	{
		return 0.0f;
	}

	// Calculate bias
	float bias = max(0.0005f * (1.0f - dot(Normal, ubo.lightDirection.xyz)), 0.00005f);
	const float biasModifier = 0.5f;
	if (layer == ubo.cascadeCount)
	{
		bias *= 1.0f / (ubo.farPlane * biasModifier);
	}
	else
	{
		bias *= 1.0f / ((ubo.cascadePlaneDistances[layer].x) * biasModifier);
	}
	bias = 0.0005f;

	float distanceToCamera = distance(ubo.viewPos.xyz, projCoords.xyz);

	bias = 0.0001f;
	float floatVal = 3.0f - (shadowsDepthMap.Sample(linearSampler, projCoords.xyz).r * 2.3f);
	int pcfCount = 5; // + int(floatVal);
	float mapSize = 4096.0f * 4.0f;
	float texelSize = (1.0f / mapSize * 2.0f) * floatVal;

	float totalTexels = (pcfCount * 2.0f + 1.0f) * (pcfCount * 2.0f + 1.0f);
	float shadow = 0.0f;

	// Percentage-closer filtering
	for (int x = -pcfCount; x <= pcfCount; ++x)
	{
		for (int y = -pcfCount; y <= pcfCount; ++y)
		{
			float2 offset = float2(x, y) * texelSize;
			float pcfDepth = shadowsDepthMap.Sample(linearSampler, float3(projCoords.xy + offset, (float)layer)).r;
			shadow += (currentDepth - bias) > pcfDepth ? 1.0f : 0.0f;
		}
	}

	shadow /= totalTexels;
	return shadow;
}
