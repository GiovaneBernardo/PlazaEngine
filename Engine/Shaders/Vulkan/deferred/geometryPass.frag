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


layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in mat4 model;

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

layout(binding = 20) uniform sampler2D textures[];

vec3 GetNormalFromMap() {
    vec3 bumpNormal = texture(textures[material.normalIndex], fragTexCoord).xyz * 2.0 - 1.0;
    return normalize(inTBN * bumpNormal);
}

void main() {
    vec3 albedo;
    if(material.diffuseIndex > -1) {
        vec4 textureColor = texture(textures[material.diffuseIndex], fragTexCoord);//pow(texture(textures[material.diffuseIndex], fragTexCoord), vec4(vec3(ubo.gamma), 1.0f));
        //vec4 textureColor = pow(texture(textures[material.diffuseIndex], fragTexCoord), vec4(vec3(1.0f / 2.2f), 1.0f));
        if(textureColor.w <= 0.05f)
            discard;
        albedo = textureColor.xyz;
    }
    else {
        albedo = material.color.xyz;
    }
    albedo *= pow(material.intensity, material.intensity);

    vec3 N = normalize(inNormal.xyz);
    if(material.normalIndex > -1) {
        N = -GetNormalFromMap();
    }

    vec3 V =  normalize(-worldPos.xyz);
    float NdotV = dot(N, V);
    if (NdotV < 0.0) {
        //N = -N;
    }

    float metallic = material.metalnessFloat;
    float roughness = material.roughnessFloat;

    if(material.metalnessIndex > -1) {
        float factor = metallic;
        metallic =  (texture(textures[material.metalnessIndex], fragTexCoord).r) * factor;
    }

    if(material.roughnessIndex > -1) {
        float factor = (roughness * 2.0f);
        roughness = texture(textures[material.roughnessIndex], fragTexCoord).r * factor;
    }

    /* Geometry */
    gOthers = vec4(0.0f, metallic, roughness, 1.0f);
    gDiffuse = vec4(albedo, 1.0f);
    gNormal = vec4(N.xyz, 1.0f);
}
