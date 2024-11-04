#version 460 core
//#extension GL_EXT_descriptor_indexing : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct MaterialData{
    vec4 color;
    float intensity;
    int diffuseIndex;
    int normalIndex;
    int roughnessIndex;
	int metalnessIndex;
    float roughnessFloat;
    float metalnessFloat;
    float flipX;
    float flipY;
};

layout(binding = 6) uniform sampler2D samplerBRDFLUT;
layout(binding = 7) uniform samplerCube prefilterMap;
layout(binding = 8) uniform samplerCube irradianceMap;
layout(binding = 9) uniform sampler2DArray shadowsDepthMap;
layout(binding = 10) uniform samplerCube equirectangularMap;
//layout(location = 10) in flat int materialIndex;
//layout (std430, set = 0, binding = 19) buffer MaterialsBuffer {
//    MaterialData materials[];
//};
layout(binding = 20) uniform sampler2D textures[];

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in mat4 model;

layout(binding = 0) uniform UniformBufferObject {
    mat4 projection;
    mat4 view;
    mat4 model;
    int cascadeCount;
    float farPlane;
    float nearPlane;
    vec4 lightDirection;
    vec4 viewPos;
    mat4 lightSpaceMatrices[16];
    vec4 cascadePlaneDistances[16];
    vec4 directionalLightColor;
    vec4 ambientLightColor;
    bool showCascadeLevels;
    float gamma;
} ubo;

layout(location = 20) in flat MaterialData material;
layout(location = 10) in vec3 FragPos;
layout(location = 11) in vec3 inNormal;
layout(location = 6) in mat3 inTBN;
layout(location = 12) in vec3 Tangent;
layout(location = 13) in vec2 TexCoords;
layout(location = 17) in vec4 worldPos;
layout(location = 18) in flat int affected;

//layout (location = 0) out vec4 gPosition;
layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gDiffuse;
layout (location = 2) out vec4 gOthers;

const float PI = 3.14159265359;
float ao = 1.0f;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

float ShadowCalculation(vec3 fragPosWorldSpace, vec3 Normal)
{
    // select cascade layer
    vec4 fragPosViewSpace = ubo.view * vec4(fragPosWorldSpace, 1.0);
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

    vec4 fragPosLightSpace = (ubo.lightSpaceMatrices[layer]) * vec4(fragPosWorldSpace.xyz, 1.0f);
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

mat3 computeTBN(vec3 normal, vec3 tangent) {
    vec3 bitangent = cross(normal, tangent);
    return mat3(tangent, bitangent, normal);
}

vec3 getNormalFromMap(vec3 N) {
    vec3 bumpNormal = texture(textures[material.normalIndex], fragTexCoord).xyz * 2.0 - 1.0;
    //bumpNormal *= -1.0;
	vec3 tangentNormal = Tangent;
    //tangentNormal.y *= -1.0;
    vec3 bitangentNormal = cross(N, tangentNormal);

    mat3 TBN = mat3(tangentNormal, bitangentNormal, bumpNormal);
    vec3 newNormal = normalize(inTBN * bumpNormal);
    return newNormal;
        
        //vec3 t0 = cross(N, vec3(1, 0, 0));
        //if(dot(t0, t0) < 0.001)
        //    t0 = cross(N, vec3(0, 1, 0));
        //t0 = normalize(t0);
        //vec3 t1 = normalize(cross(N, t0));
        //vec3 newNormal = normalize(mat3(t0, t1, N) * bumpNormal);
        //return newNormal;

	////vec3 N = normalize(inNormal);
	//vec3 T = normalize(Tangent.xyz);
	//vec3 B = normalize(cross(N, T));
	//mat3 TBN = mat3(T, B, N);
	//return normalize(TBN * tangentNormal);
}

vec3 F_Schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 prefilteredReflection(vec3 R, float roughness) {
	const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	vec3 a = textureLod(prefilterMap, R, lodf).rgb;
	vec3 b = textureLod(prefilterMap, R, lodc).rgb;
	return mix(a, b, lod - lodf);
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

void main() {
    //MaterialData material = materials[materialIndex];
    float gammaCorection = 1.0f / pow(2.2f, ubo.gamma) * 3.0f;
    vec3 albedo;
    if(material.diffuseIndex > -1) { 
        vec4 textureColor = texture(textures[material.diffuseIndex], fragTexCoord);
        //vec4 textureColor = pow(texture(textures[material.diffuseIndex], fragTexCoord), vec4(vec3(1.0f / 2.2f), 1.0f));
        if(textureColor.w <= 0.1f)
            discard;
        albedo = textureColor.xyz;
    }
    else {
        albedo = material.color.xyz;
    }

    vec3 N = normalize(inNormal.xyz);
    if(material.normalIndex > -1) {
        N = getNormalFromMap(N);
    }

    //N = Tangent;
    float metallic = 0.98f - min(material.metalnessFloat, 0.98f);
    float roughness = min(material.roughnessFloat, 0.98f);

    if(material.metalnessIndex > -1) {
        metallic =  texture(textures[material.metalnessIndex], fragTexCoord).r; //* (metallic * 2.0f);
    }

    if(material.roughnessIndex > -1) {
        roughness = texture(textures[material.roughnessIndex], fragTexCoord).r;//* (roughness * 2.0f);
    }

    vec3 viewP = ubo.viewPos.xyz;
    //viewP.y = -1.0f;
    vec3 V =  normalize(viewP - (worldPos.xyz));
    vec3 Ve = V;
    //Ve.y = 1.0f - Ve.y;
    float NdotV = dot(N, Ve);
    //NdotV = 1.0f - NdotV;
    if (NdotV < 0.0) {
        N = -N;
        NdotV = 1.0f - NdotV;
    }
    //NdotV = clamp(NdotV, -0.5f, 0.5f);
    vec3 R = reflect(-V, N); 

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    float maxNdotV = max(NdotV, 0.25);
    vec3 F = F_SchlickR(maxNdotV, F0, roughness);

    vec3 kD = (1.0 - F) * (1.0 - metallic);

    vec3 L = normalize(ubo.lightDirection.xyz); // Directional light direction

    vec3 Lo = specularContribution(L, V, N, F0, metallic, roughness, albedo);

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo.xyz ;

    const float MAX_REFLECTION_LOD = 9.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;   
    vec2 brdfCoord = vec2(maxNdotV, roughness);
    vec2 brdf  = texture(samplerBRDFLUT, brdfCoord).rg;
    vec3 reflection = prefilteredReflection(R, roughness).rgb;	
    vec3 specular = reflection * ((F * brdf.x + brdf.y));

    float ambientOcclusion = 1.0f;
    vec3 ambient = (kD * diffuse + specular) * ambientOcclusion;
    //ambient *= material.intensity;

    vec3 shadow = (1.0f - ShadowCalculation(FragPos.xyz, N)) * ubo.directionalLightColor.xyz;
    vec3 color = ambient; //+ ubo.directionalLightColor.xyz; // Directional Light
     color *= Lo * shadow + ubo.ambientLightColor.xyz;

    color *= pow(material.intensity, material.intensity);

     //color = vec3(1.0 - ShadowCalculation(FragPos.xyz, vec3(1.0f)));
    vec3 FinalColor = color;

    #ifdef SHOW_CASCADES
    if(ubo.showCascadeLevels) { 
        vec4 fragPosViewSpace = ubo.view * vec4(FragPos.xyz, 1.0);
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
        FinalColor = vec3(1.0f - float(layer) / float(ubo.cascadeCount), 0.0f, 0.0f);
    }
    #endif


    /* Geometry */
    gOthers = vec4(specular.x, material.metalnessIndex > -1 ? 1.0f - metallic : metallic, roughness, 1.0f);
    //gPosition = vec4(worldPos);
    gDiffuse = vec4(FinalColor, 1.0f);

    if(material.normalIndex > -1) {
        //gNormal = vec4(texture(textures[material.normalIndex], fragTexCoord).xyz, 1.0f);
        gNormal = vec4(N.xyz, 1.0f);
    } else {
        gNormal = vec4(N.xyz, 1.0f);
    }
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

layout(push_constant) uniform PushConstants {
    vec4 color;
    float intensity;
    int diffuseIndex;
    int normalIndex;
    int roughnessIndex;
	int metalnessIndex;
    float roughnessFloat;
    float metalnessFloat;
} pushConstants;