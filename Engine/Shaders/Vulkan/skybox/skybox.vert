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
layout(location = 8) in uint vertexMaterialIndex;
//layout(location = 12) in uint materialsOffsets[64];

layout(location = 0) out vec3 fragTexCoord;

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

out gl_PerVertex {
	vec4 gl_Position;
};

layout(push_constant) uniform PushConstants{
    mat4 projection;
    mat4 view;
	float skyboxIntensity;
	float gamma;
	float exposure;
    float useless;
} pushConstants;

#define USE_SPHERE

void main()
{
#ifdef USE_CUBE
	fragTexCoord.xyz = inPosition.xyz;
#endif
#ifdef USE_SPHERE
    fragTexCoord.xyz = inPosition.xyz;
    //fragTexCoord.xy = inPosition.xy;
    //fragTexCoord.x = fragTexCoord.x - 0.5f;
    //fragTexCoord.y = 1.0f - fragTexCoord.y;
#endif


	mat4 viewMat = mat4(mat3(pushConstants.view));
	gl_Position = pushConstants.projection * viewMat * vec4(inPosition.xyz, 1.0f);
	gl_Position.z = gl_Position.w;
}
