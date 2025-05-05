#version 450
#extension GL_EXT_nonuniform_qualifier : enable

struct LightStruct {
    vec3 color;
    float radius;
    vec3 position;
    float intensity;
    float cutoff;
    float minRadius;
};

struct Cluster{
    int[256] lightsIndex;
    vec3 minBounds;
    int lightsCount;
    vec3 maxBounds;
    int alignment;
};

struct Plane
{
    vec3 Normal;
    float Distance;
};

struct Frustum
{
    Plane planes[4];
};

layout (std430, binding = 4) buffer LightsBuffer {
    LightStruct lights[];
};

layout (std430, binding = 5) buffer ClusterBuffer {
    Cluster[] clusters;
};

layout(binding = 15) uniform UniformBufferObject {
    mat4 projection;
    mat4 view;
    bool showCascadeLevels;
    float farPlane;
    float nearPlane;
    float gamma;
    float exposure;
    int cascadeCount;
    int lightCount;
    vec4 viewPos;
    vec4 lightDirection;
    vec4 ambientLightColor;
    vec4 directionalLightColor;
    mat4 lightSpaceMatrices[16];
    vec4 cascadePlaneDistances[16];
} ubo;

layout (binding = 0) uniform sampler2D gNormal;
layout (binding = 1) uniform sampler2D gDiffuse;
layout (binding = 2) uniform sampler2D gOthers;
layout (binding = 3) uniform sampler2D gSceneDepth;

layout(binding = 6) uniform sampler2D samplerBRDFLUT;
layout(binding = 7) uniform samplerCube prefilterMap;
layout(binding = 8) uniform samplerCube irradianceMap;
layout(binding = 9) uniform sampler2DArray shadowsDepthMap;
layout(binding = 10) uniform samplerCube equirectangularMap;

layout(location = 0) out vec4 FragColor;
layout(location = 1) in vec2 TexCoords;

vec2 screenSize = vec2(1820, 720);
vec3 clusterSize = vec3(32, 32, 32);

vec4 HeatMap(int clusterIndex, int numLights)
{
    vec3 color;
    if(numLights <= 0)
    {
        color = vec3(0.5f, 0.5f, 0.5f);
    }
    else if(numLights < 10)
    {
        color = vec3(0.2f, 1.0f, 0.0f);
    }
    else if(numLights < 50)
    {
        color = vec3(0.4f, 0.8f, 0.0f);
    }
    else if(numLights < 100)
    {
        color = vec3(0.8f, 0.4f, 0.0f);
    }
    else if(numLights >= 100)
    {
        color = vec3(1.0f, 0.0f, 0.0f);
    }
    return vec4(color, 1.0f);
}

float roundToMultiple(float value, float multiple) {
    return floor(value / multiple) * multiple;
}

int GetIndex(vec2 screenPos, vec2 clusterCount)
{
    vec2 currentClusterPosition = vec2(ceil(roundToMultiple(screenPos.x * screenSize.x, clusterSize.x) / clusterSize.x), ceil(roundToMultiple(screenPos.y * screenSize.y, clusterSize.y) / clusterSize.y));
    return int(((currentClusterPosition.y * (clusterCount.x)) + (currentClusterPosition.x)));
}

const float PI = 3.14159265359;

#define MAX_POINT_LIGHT_PER_TILE 2048
//#define SHOW_HEATMAP

float linearizeDepth(float depth) {
    float far = 15000.0f;
    float near = 0.01f;
    return (2.0 * near * far) / (far + near - depth * (far - near));
}

float GetDepth() {
    return texture(gSceneDepth, TexCoords).r;
}

vec3 ReconstructPosition(float depth) {
    mat4 viewProjection = ubo.projection * ubo.view;
    float ndcZ = (depth) * 2.0f - 1.0f;
    vec4 ndcPosition = vec4(gl_FragCoord.xy * (1.0f / screenSize) * 2.0 - 1.0, (depth), 1.0);
    ndcPosition.y *= -1;
    vec4 worldPosition = inverse(viewProjection) * ndcPosition;
    worldPosition.xyz /= worldPosition.w;
    return worldPosition.xyz;
}

vec3 ReconstructViewPosition(vec2 uv, float depth) {
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    clipPos.y *= -1.0f;
    vec4 viewPos = inverse(ubo.projection) * clipPos;
    return viewPos.xyz / viewPos.w;
}

//vec3 CalculatePointLight() {
//    return vec3(1.0f);
//}

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

vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 prefilteredReflection(vec3 R, float roughness) {
	const float MAX_REFLECTION_LOD = 9.0;
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	vec3 a = textureLod(prefilterMap, R, lodf).rgb;
	vec3 b = textureLod(prefilterMap, R, lodc).rgb;
	return mix(a, b, lod - lodf);
}

vec3 F_Schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness, vec3 materialColor)
{
    vec3 H = normalize (V + L);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);

    vec3 color = vec3(0.0);

    if (dotNL > 0.0) {
        float D = D_GGX(dotNH, roughness);
        float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
        vec3 F = F_Schlick(dotNV, F0);
        vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
        color += (kD * materialColor / PI + spec) * dotNL;
    }

    return color;
}

vec3 ViewToWorldSpace(vec3 viewPosition) {
    // Use the inverse of the view matrix
    vec4 worldPosition = inverse(ubo.view) * vec4(viewPosition, 1.0);
    return worldPosition.xyz; // Convert back to vec3
}

float ShadowCalculation(vec3 fragPos, vec3 Normal)
{
    // select cascade layer
    vec4 fragPosViewSpace = vec4(fragPos, 1.0f);//ubo.view * vec4(fragPosWorldSpace, 1.0);
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

    vec4 fragPosLightSpace = (ubo.lightSpaceMatrices[layer]) * vec4(ViewToWorldSpace(fragPos.xyz), 1.0f);
    // perform perspective divide
    vec4 projCoords = fragPosLightSpace / fragPosLightSpace.w;
    float currentDepth = projCoords.z;
    projCoords = projCoords * 0.5 + 0.5;
    projCoords.y = 1 - projCoords.y;

    float shadowFromDepthMap = texture( shadowsDepthMap, vec3(projCoords.xy, float(layer))).r;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0)
    {
        return 0.0f;
    }
    // calculate bias (based on depth map resolution and slope)
    float bias = max(0.0005 * (1.0 - dot(Normal, ubo.lightDirection.xyz)), 0.00005);
    const float biasModifier = 0.5f;
    if (layer == ubo.cascadeCount)
    {
        bias *= 1 / (ubo.farPlane * biasModifier);
    }
    else
    {
        bias *= 1 / ((ubo.cascadePlaneDistances[layer].x) * biasModifier);
    }
    bias = 0.0005;
    float distanceToCamera = distance(ubo.viewPos.xyz, projCoords.xyz);

    bias = 0.0001;
    float floatVal = 3 - (texture(shadowsDepthMap, vec3(projCoords.xyz)).r * 2.3);
    int pcfCount = 5;// + int(floatVal);
    float mapSize = 4096.0 * 4;
    float texelSize = (1.0 / mapSize * 2) * floatVal;
    float total = 0.0;
    float totalTexels = (pcfCount * 2.0 + 1.0) * (pcfCount * 2.0 + 1.0);
    // PCF
    float shadow = 0.0;
    //vec2 texelSize = 1.0 / vec2(textureSize(shadowsDepthMap, 0));
    for(int x = -pcfCount; x <= pcfCount; ++x)
    {
        for(int y = -pcfCount; y <= pcfCount; ++y)
        {
            float pcfDepth = texture(shadowsDepthMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= totalTexels;
    return shadow;
}

vec3 CalculateDirectionalLight(vec3 fragPos, vec3 albedo, vec3 normal, float metallic, float roughness) {
    vec3 V =  normalize(-fragPos);
    float NdotV = dot(normal, V);
    if (NdotV < 0.0) {
        normal = -normal;
        NdotV = 1.0f - NdotV;
    }

    vec3 R = mat3(inverse(ubo.view)) * reflect(-V, normal);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    float maxNdotV = max(NdotV, 0.25);
    vec3 F = F_SchlickR(maxNdotV, F0, roughness);

    vec3 kD = (1.0 - F) * (1.0 - metallic);

    vec3 L = normalize((ubo.view * vec4(ubo.lightDirection.xyz, 0.0)).xyz);

    vec3 Lo = specularContribution(L, V, normal, F0, metallic, roughness, albedo);

    vec3 irradiance = texture(irradianceMap, normal).rgb + ubo.ambientLightColor.x;
    vec3 diffuse = irradiance * albedo.xyz;

    const float MAX_REFLECTION_LOD = 9.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdfCoord = vec2(maxNdotV, roughness);
    vec2 brdf  = texture(samplerBRDFLUT, brdfCoord).rg;
    vec3 reflection = prefilteredReflection(R, roughness).rgb;
    vec3 specular = reflection * ((F * brdf.x + brdf.y)) ;

    float ambientOcclusion = 1.0f;
    vec3 ambient = (kD * diffuse + specular) * ambientOcclusion;

    vec3 shadow = (1.0f - ShadowCalculation(fragPos.xyz, normal)) * (ubo.directionalLightColor.xyz) + ubo.ambientLightColor.x;
    shadow = vec3(1.0f);
    vec3 color = ambient;
    color += Lo;
    color *= shadow;
    return color * vec3(ubo.directionalLightColor.w);
}

vec3 CalculatePointLight(vec3 fragPos, vec3 albedo, vec3 normal, float roughness, vec3 lightPos, vec3 lightColor, float metallic, float attenuation) {
    vec3 V = normalize(-fragPos);        // View direction
    vec3 L = normalize(lightPos - fragPos);           // Light direction
    float NdotL = max(dot(normal, L), 0.0);           // Lambertian diffuse

    // Fresnel reflectance factor for metallic surfaces
    vec3 F0 = mix(vec3(0.04), albedo, metallic);      // Base reflectance
    vec3 F = F_SchlickR(NdotL, F0, roughness);        // Fresnel term
    vec3 kD = (1.0 - F) * (1.0 - metallic);           // Diffuse weight

    // Diffuse component
    vec3 diffuse = (kD * albedo) * lightColor * NdotL;  // Basic diffuse shading
    //diffuse *= 1.0f;

    //return vec3(normal);
    vec3 color = diffuse + (specularContribution(L, V, normal, F0, metallic, roughness, albedo) * lightColor * 5.0f ); // or color = ambient + diffuse + specular if those terms are used
    return color * attenuation;
}

float CalculateAttenuation(vec3 lightPos, vec3 fragPos, float minRadius, float maxRadius) {
    float distance = length(lightPos - fragPos);

    // Compute linear attenuation based on distance and clamp within min/max range
    float attenuation = clamp(1.0 - (distance - minRadius) / (maxRadius - minRadius), 0.0, 1.0);

    // Optionally, square the attenuation to make the falloff effect smoother
    attenuation *= attenuation;

    return attenuation;
}

void main()
{
    float depth = GetDepth();
    vec3 lighting  = vec3(1.0f);

    vec2 clusterCount = ceil(screenSize / clusterSize.xy);
    int clusterIndex = GetIndex(TexCoords, clusterCount);
    Cluster currentCluster = clusters[clusterIndex];
    const vec3 Diffuse = texture(gDiffuse, TexCoords).xyz;
    vec3 color = Diffuse;
    //if(ubo.lightCount > 0 && depth != 1.0f) {
    if(depth != 1.0f) {
        const vec3 fragViewPos = ReconstructViewPosition(TexCoords, depth);//ReconstructPosition(depth);
        const vec3 Normal = texture(gNormal, TexCoords).rgb;
        const vec3 Others = texture(gOthers, TexCoords).xyz;
        const float Specular = Others.x;
        const float metalness = Others.y;
        const float roughness = Others.z;
        color = vec3(0.0f);
        color = CalculateDirectionalLight(fragViewPos, Diffuse, Normal, metalness, roughness);
        //lighting = CalculateDirectionalLight(FragPos, Diffuse, Normal, metalness, roughness, shadow);
        //lighting = vec3(1.0f);//CalculateDirectionalLight(FragPos, Diffuse, Normal, metalness, roughness, shadow);
       // float shadow = (1.0f - ShadowCalculation(FragPos.xyz, Normal)); //* ubo.directionalLightColor.xyz;
       // lighting += CalculateDirectionalLight(FragPos, Diffuse, Normal, metalness, roughness, shadow);
        if(ubo.lightCount > 0) {
            for (int i = 0; i < MAX_POINT_LIGHT_PER_TILE && clusters[clusterIndex].lightsIndex[i] != -1; ++i) {
                LightStruct light = lights[clusters[clusterIndex].lightsIndex[i]];
                vec3 lightPosition = vec3(ubo.view * vec4(light.position, 1.0f)); //vec3(pushConstants.view * vec4(light.position, 1.0));

                vec3 lightColor = light.color * light.intensity;
                float attenuation =  CalculateAttenuation(lightPosition, fragViewPos, light.cutoff, light.radius + light.cutoff);
                lighting += (CalculatePointLight(fragViewPos, Diffuse, Normal, roughness, lightPosition, lightColor, metalness, attenuation) * 1.0f);//atten;//Lo;
            }
         }
        color = color + (lighting - 1.0f); //* (lighting);
    }

    //color += max((lighting - 1.0f) * Diffuse, vec3(0.0f));

//    #define SHOW_HEATMAP
//#ifdef SHOW_HEATMAP
//    vec3 heatmap = HeatMap(clusterIndex, currentCluster.lightsCount).xyz;
//    color = (color + heatmap) * 0.5f;
//#endif

    FragColor = vec4(color, 1.0f);
}
