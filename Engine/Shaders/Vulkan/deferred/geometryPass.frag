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

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 9) uniform sampler2DArray shadowsDepthMap;
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
layout(location = 11) in vec4 FragPos;
layout(location = 12) in vec4 Normal;
layout(location = 13) in vec2 TexCoords;
layout(location = 14) in vec4 TangentLightPos;
layout(location = 15) in vec4 TangentViewPos;
layout(location = 16) in vec4 TangentFragPos;
layout(location = 17) in vec4 worldPos;
layout(location = 18) in flat int affected;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gDiffuse;
layout (location = 3) out vec4 gOthers;

const float PI = 3.14159265359;
float ao = 0;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

float ShadowCalculation(vec3 fragPosWorldSpace)
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
    vec3 normale;
    bool usingNormal = false;
    if(usingNormal){
        normale = texture(textures[material.normalIndex], TexCoords).rgb;
        normale = normalize(normale * 2.0 - 1.0);  // this normal is in tangent space
    } else {
        normale = normalize(Normal.xyz);
    }
    float bias = max(0.0005 * (1.0 - dot(normale, ubo.lightDirection.xyz)), 0.00005);
    const float biasModifier = 0.5f;
    if (layer == ubo.cascadeCount)
    {
        bias *= 1 / (ubo.farPlane * biasModifier);
    }
    else
    {
        bias *= 1 / ((ubo.cascadePlaneDistances[layer].x) * biasModifier);
    }
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

float intensity = 1.0f;
vec3 schlickFresnel(float vDotH, vec3 color)
{
    vec3 F0 = vec3(0.04);
    F0 = color;

    vec3 ret = F0 + (1 - F0) * pow(clamp(1.0 - vDotH, 0.0, 1.0), 5);

    return ret;
}


float geomSmith(float dp, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float denom = dp * (1 - k) + k;
    return dp / denom;
}


float ggxDistribution(float nDotH, float roughness)
{
    float alpha2 = roughness * roughness * roughness * roughness;
    float d = nDotH * nDotH * (alpha2 - 1) + 1;
    float ggxdistrib = alpha2 / (PI * d * d);
    return ggxdistrib;
}

void main() {
    //MaterialData material = materials[materialIndex];
    vec4 color;
    if(material.diffuseIndex > -1)
    { 
        color = texture(textures[material.diffuseIndex], fragTexCoord);
        if(color.w <= 0.1f)
            discard;
        color.xyz *= material.intensity;
    }
    else
    {
        color = vec4(material.color.xyz, 1.0f) * material.intensity;
    }

    //color *= vec4((vec3(1.0f) - ShadowCalculation(FragPos.xyz) + 0.25f).xyz, 1.0f);

    color = color * 1;
    //color.xyz = pow(color, vec3(1.0f / ubo.gamma))

    vec3 lightColor = ubo.directionalLightColor.xyz * 255;//vec3(1.0f, 0.85f, 0.85f) * 255;
    // ambient
    vec3 ambient = 1.32 * (lightColor / 1);
    // diffuse

    float metallic = material.metalnessFloat;
    float roughness = material.roughnessFloat;

    if(material.metalnessIndex > -1)
    {
        metallic =  texture(textures[material.metalnessIndex], fragTexCoord).r;//pow(texture(textures[pushConstants.metalnessIndex], fragTexCoord) / 1, vec4(1/ 2.2)).r * 1;
    }

    if(material.roughnessIndex > -1)
    {
        roughness = texture(textures[material.roughnessIndex], fragTexCoord).r;// pow(texture(textures[pushConstants.roughnessIndex], fragTexCoord) / 1, vec4(1/ 2.2)).r * 1;
    }

    //metallic = texture(texture_metalness, fs_in.TexCoords).r / 255;//pow(texture(texture_metalness, fs_in.TexCoords), vec4(2.2)).r;
    //roughness = texture(texture_metalness, fs_in.TexCoords).r / 255;//pow(texture(texture_roughness, fs_in.TexCoords) / 1, vec4(2.2)).r;
    //metallic *= 2;
    //metallic = 0.5 * 1;
    //roughness = 0.3f * 1;
    vec3 normal;
    vec3 lightDir;
    float diff;
    vec3 diffuse;
    vec3 viewDir;

    bool usingNormal = false;
    if(usingNormal){
        normal = texture(textures[material.normalIndex], fragTexCoord).rgb;
        normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
        //lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
        diff = max(dot(ubo.lightDirection.xyz, normal), 0.0);
        viewDir = normalize(TangentViewPos.xyz - TangentFragPos.xyz);
    } else {
        normal = normalize(Normal.xyz);
        //lightDir = normalize(lightPos - fs_in.FragPos);
        diff = max(dot(ubo.lightDirection.xyz, normal), 0.0);
        viewDir = normalize(ubo.viewPos.xyz - FragPos.xyz);
    }

    vec3 l = normalize(ubo.lightDirection.xyz);
    vec3 n = normal;
    vec3 v = normalize(ubo.viewPos.xyz - worldPos.xyz);
    vec3 h = normalize(v + l);

    float nDotH = max(dot(n, h), 0.0);
    float vDotH = max(dot(v, h), 0.0);
    float nDotL = max(dot(n, l), 0.0);
    float nDotV = max(dot(n, v), 0.0);

    vec3  F0 = mix (vec3 (0.04), color.xyz, metallic);
    vec3 F = fresnelSchlick(vDotH, F0);   

    vec3 kS = F;
    vec3 kD = 1.0 - kS;

    vec3 SpecBRDF_nom  = ggxDistribution(nDotH, roughness) *
                         F *
                         geomSmith(nDotL, roughness) *
                         geomSmith(nDotV, roughness);

    float SpecBRDF_denom = 4.0 * nDotV * nDotL + 0.0001;

    vec3 SpecBRDF = SpecBRDF_nom / SpecBRDF_denom;

    vec3 fLambert = vec3(0.0);
    fLambert = color.xyz * 1;

    vec3 DiffuseBRDF = kD * fLambert / PI;

    float shadow = (1 - ShadowCalculation(FragPos.xyz)) * 2;
    vec3 amb = ubo.ambientLightColor.xyz;//vec3(0.72f);

    float specularIntensity = 13.0f;


    gOthers = vec4(SpecBRDF * specularIntensity, 1.0f);
    gOthers.z = metallic;

    vec3 FinalColor = (kD * color.xyz + kS * SpecBRDF) * (max(nDotL, amb.x / 2) * (amb + (ubo.directionalLightColor.xyz * shadow)));

    vec3 dif2 = color.xyz;
    vec3 spec2 = vec3(0.0f);

    kD = 1.0f - F;
    kD *= 1.0f - metallic;
    FinalColor = (kD * dif2 + spec2) * ((ubo.directionalLightColor.xyz * shadow) + ubo.ambientLightColor.xyz);//(max(nDotL, amb.x / 2) * (amb + (ubo.directionalLightColor.xyz * shadow)));
    //vec3 fog = vec3(0.7f, 0.7f, 0.0f) * pow(distance(FragPos, ubo.viewPos) / 15000.0f, 0.5f);
    //FinalColor *= vec3(1.0f) - fog;

    /* Geometry */
      gPosition = vec4(FragPos);
      gDiffuse = vec4(FinalColor, 1.0f);

      if(usingNormal)
      {
          vec3 T = normalize(TangentLightPos - TangentFragPos).xyz;
          vec3 B = cross(normal, T);
          mat3 TBN = mat3(T, B, normal);
          vec3 normalB = normalize(TBN * normal);
          // Transform normal to world space
          //normalB = normalize((view * model * vec4(normal, 0.0)).xyz);
          vec3 normalWorld = normalize((ubo.view * model * vec4(normalB, 1.0f)).xyz);
          gNormal = vec4(normalWorld, 1.0f);
      }
      else
          gNormal = vec4(normalize(normal), 1.0f);

    //if(affected == 1)
    //    gDiffuse = vec4(1.0f, 0.0f, 0.0f, 1.0f);

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
