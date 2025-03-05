#version 460 core
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
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec4 instanceMatrix[4];
layout(location = 8) in ivec4 boneIds;
layout(location = 9) in vec4 weights;
layout(location = 10) in uint vertexMaterialIndex;
//layout(location = 12) in uint materialsOffsets[64];

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out mat4 model;

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
layout (std430, set = 0, binding = 19) buffer MaterialsBuffer {
    MaterialData materials[];
};

const int MAX_BONES = 1000;
const int MAX_BONE_INFLUENCE = 4;
layout(std430, binding = 1) readonly buffer BoneMatrices {
	mat4 boneMatrices[];
};

//layout(std430, binding = 2) readonly buffer RenderGroups {
//    uint renderGroupOffsets[]; // This takes the instance id, returns an index that will be the starting point of the material offsets, thus converting the vertex material index (from 0 to ...) into an index that takes into account prior materials
//	uint renderGroupMaterialsOffsets[]; // This takes the converted vertex material index and returns the real material index
//};

layout(std430, binding = 2) readonly buffer RenderGroupOffsetsBuffer {
    uint renderGroupOffsets[];
};

layout(std430, binding = 3) readonly buffer RenderGroupMaterialsOffsetsBuffer {
	uint renderGroupMaterialsOffsets[];
};

//#define MESH_RENDERER_MAX_MATERIALS 64
//layout(location = 18) in uint[MESH_RENDERER_MAX_MATERIALS] meshRendererMaterials;
//layout(location = 18) in uint meshRendererMaterials;
//layout(location = 18) in uint mat;
layout(binding = 20) uniform sampler2D textures[];

layout(location = 20) out MaterialData material;
layout(location = 10) out vec3 FragPos;
layout(location = 11) out vec3 Normal;
layout(location = 12) out vec3 Tangent;
layout(location = 13) out vec2 TexCoords;
layout(location = 17) out vec4 worldPos;
layout(location = 18) out flat int affected;
layout(location = 6) out mat3 outTBN;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
// TODO: FIX ANIMATION NORMALS
    model = mat4(instanceMatrix[0], instanceMatrix[1], instanceMatrix[2], instanceMatrix[3]);

    mat4 aInstanceMatrix = model;

    mat4 finalInstanceMatrix = model;
    FragPos = vec3(model * vec4(inPosition, 1.0));

    vec4 totalPosition = vec4(0.0f);
    bool allNegative = true;

    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
          if(boneIds[i] < 0 || boneIds[i] > 1000)
          {
            continue;
          }
          else
          {
               allNegative = false;
          }
          if(boneIds[i] >= MAX_BONES)
          {
              totalPosition = vec4(inPosition, 1.0f);
              break;
          }
          vec4 localPosition = boneMatrices[boneIds[i]] * vec4(inPosition,1.0f);
          totalPosition += localPosition * weights[i];
          vec3 localNormal = mat3(boneMatrices[boneIds[i]]) * Normal.xyz;
    }

     if(allNegative)
         totalPosition = vec4(inPosition,1.0f);

    vec4 finalModel = finalInstanceMatrix * totalPosition;

    worldPos.xyz = vec3(finalModel);
    gl_Position = ubo.projection * ubo.view * finalModel;

    Normal.xyz = normalize(mat3(ubo.view) * mat3(finalInstanceMatrix) * inNormal);
    // Calculate tangent and bitangent
    vec3 edge1 = vec3(finalInstanceMatrix * vec4(inPosition + vec3(1.0, 0.0, 0.0), 1.0)) - vec3(finalModel);
    vec3 edge2 = vec3(finalInstanceMatrix * vec4(inPosition + vec3(0.0, 1.0, 0.0), 1.0)) - vec3(finalModel);
    vec2 deltaUV1 = inTexCoord + vec2(1.0, 0.0) - inTexCoord;
    vec2 deltaUV2 = inTexCoord + vec2(0.0, 1.0) - inTexCoord;

    float f = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
    vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
    vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);

    tangent = normalize(tangent);
    bitangent = normalize(bitangent);

    // Set the TBN matrix
    outTBN = mat3(tangent, bitangent, Normal);

vec3 viewNormal = Normal;
vec3 viewTangent = normalize(mat3(ubo.view) * tangent);
vec3 viewBitangent = normalize(mat3(ubo.view) * bitangent);

// Construct the TBN matrix in view space
outTBN = mat3(viewTangent, viewBitangent, viewNormal);

    uint materialIndex = renderGroupMaterialsOffsets[vertexMaterialIndex + renderGroupOffsets[gl_InstanceIndex]];//meshRendererMaterials[vertexMaterialIndex];//22;

    fragTexCoord = inTexCoord;
    fragTexCoord.y = 1.0f - fragTexCoord.y;//vec2(materials[materialIndex].flipX, materials[materialIndex].flipY) - inTexCoord;
    material = materials[materialIndex];
}
