#pragma extension SPV_KHR_multiview
#pragma extension SPV_EXT_descriptor_indexing

#define MAX_BONES 1000
#define MAX_BONE_INFLUENCE 4

struct UniformBufferObject
{
	column_major float4x4 projection;
	column_major float4x4 view;
	column_major float4x4 model;
	int cascadeCount;
	float farPlane;
	float nearPlane;
	float4 lightDirection;
	float4 viewPos;
	column_major float4x4 lightSpaceMatrices[16];
	float4 cascadePlaneDistances[16];
	float4 directionalLightColor;
	float4 ambientLightColor;
	bool showCascadeLevels;
	float gamma;
};

cbuffer UBO : register(b0)
{
	UniformBufferObject ubo;
}

struct MaterialData
{
	float4 color;
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
[[vk::binding(18, 0)]]
StructuredBuffer<MaterialData> MaterialsSSBO;

[[vk::binding(1, 0)]]
StructuredBuffer<float4x4> BoneMatrices;

[[vk::binding(2, 0)]]
StructuredBuffer<uint> RenderGroupOffsets;

[[vk::binding(3, 0)]]
StructuredBuffer<uint> RenderGroupMaterialsOffsets;

struct VSInput
{
	float3 inPosition      : POSITION0;   // location 0
	float3 inNormal        : NORMAL0;     // location 1
	float2 inTexCoord      : TEXCOORD0;   // location 2
	float3 inTangent       : TEXCOORD1;   // location 3
	float4 instanceMatrix0 : TEXCOORD2;   // location 4
	float4 instanceMatrix1 : TEXCOORD3;
	float4 instanceMatrix2 : TEXCOORD4;
	float4 instanceMatrix3 : TEXCOORD5;
	uint vertexMaterialIdx : TEXCOORD6;   // location 8
};

// Output to pixel shader
struct VSOutput
{
	float4 fragColor    : TEXCOORD0;
	float2 fragTexCoord : TEXCOORD1;
	column_major float4x4 model      : TEXCOORD2;
	MaterialData material : TEXCOORD20;
	float3 FragPos      : TEXCOORD10;
	float3 Normal       : TEXCOORD11;
	float3 Tangent      : TEXCOORD12;
	float2 TexCoords    : TEXCOORD13;
	float4 worldPos     : TEXCOORD17;
	nointerpolation int affected : TEXCOORD18;
	float3x3 outTBN     : TEXCOORD6;
	float4 pos          : SV_POSITION;
};

// Main VS
VSOutput mainVS(VSInput input, uint instanceID : SV_InstanceID)
{
    VSOutput output;

	float4x4 modelMat = float4x4(
		float4(input.instanceMatrix0.x, input.instanceMatrix1.x, input.instanceMatrix2.x, input.instanceMatrix3.x),
		float4(input.instanceMatrix0.y, input.instanceMatrix1.y, input.instanceMatrix2.y, input.instanceMatrix3.y),
		float4(input.instanceMatrix0.z, input.instanceMatrix1.z, input.instanceMatrix2.z, input.instanceMatrix3.z),
		float4(input.instanceMatrix0.w, input.instanceMatrix1.w, input.instanceMatrix2.w, input.instanceMatrix3.w)
	);

	output.model = modelMat;
	output.FragPos = mul(modelMat, float4(input.inPosition, 1.0f)).xyz;

	float4 finalModel = mul(modelMat, float4(input.inPosition, 1.0f));
	output.worldPos = finalModel;

	// Position
	output.pos = mul(ubo.projection, mul(ubo.view, finalModel));

	// Normal
	output.Normal = normalize(mul((float3x3)ubo.view, mul((float3x3)modelMat, input.inNormal)));

	// Tangent basis
	float3 edge1 = mul(modelMat, float4(input.inPosition + float3(1,0,0),1)).xyz - finalModel.xyz;
	float3 edge2 = mul(modelMat, float4(input.inPosition + float3(0,1,0),1)).xyz - finalModel.xyz;

	float2 deltaUV1 = float2(1,0);
	float2 deltaUV2 = float2(0,1);

	float f = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	float3 tangent   = normalize(f * (deltaUV2.y * edge1 - deltaUV1.y * edge2));
	float3 bitangent = normalize(f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2));

	float3 viewNormal   = output.Normal;
	float3 viewTangent  = normalize(mul((float3x3)ubo.view, tangent));
	float3 viewBitangent= normalize(mul((float3x3)ubo.view, bitangent));
	output.outTBN = float3x3(viewTangent, viewBitangent, viewNormal);

	// Material fetch
	uint matIndex = RenderGroupMaterialsOffsets[input.vertexMaterialIdx + RenderGroupOffsets[instanceID]];
	output.material = MaterialsSSBO[matIndex];

	// Texcoords
	output.fragTexCoord = input.inTexCoord;
	output.fragTexCoord.y = 1.0f - output.fragTexCoord.y;

	return output;
}

// Pixel Shader
[[vk::binding(20, 0)]]
Texture2D<float4> textures[];

[[vk::binding(19, 0)]]
SamplerState texSampler;

struct PSInput
{
	float4 fragColor    : TEXCOORD0;
	float2 fragTexCoord : TEXCOORD1;
	column_major float4x4 model      : TEXCOORD2;
	MaterialData material : TEXCOORD20;
	float3 FragPos      : TEXCOORD10;
	float3 Normal       : TEXCOORD11;
	float3 Tangent      : TEXCOORD12;
	float2 TexCoords    : TEXCOORD13;
	float4 worldPos     : TEXCOORD17;
	nointerpolation int affected : TEXCOORD18;
	float3x3 inTBN      : TEXCOORD6;
	float4 pos          : SV_POSITION;
};

struct PSOutput
{
	float4 gNormal  : SV_Target0;
	float4 gDiffuse : SV_Target1;
	float4 gOthers  : SV_Target2;
	float  gDepth   : SV_Target3;
};

float3 GetNormalFromMap(PSInput input)
{
	float3 bumpNormal = textures[input.material.normalIndex].Sample(texSampler, input.fragTexCoord).xyz * 2.0 - 1.0;
	return normalize(mul(input.inTBN, bumpNormal));
}

PSOutput mainPS(PSInput input)
{
	PSOutput output;

	float3 albedo;
	if (input.material.diffuseIndex > -1)
	{
		float4 texColor = textures[input.material.diffuseIndex].Sample(texSampler, input.fragTexCoord);
		if (texColor.a <= 0.05f) discard;
		albedo = texColor.xyz;
	}
	else
	{
		albedo = input.material.color.xyz;
	}
	albedo *= pow(input.material.intensity, input.material.intensity);

	// Normal
	float3 N = normalize(input.Normal);
	if (input.material.normalIndex > -1)
		N = -GetNormalFromMap(input);

	// View vector
	float3 V = normalize(-input.worldPos.xyz);
	float NdotV = dot(N, V);
	if (NdotV < 0.0f) {
		// optionally flip normal
	}

	// Metal/roughness
	float metallic  = input.material.metalnessFloat;
	float roughness = input.material.roughnessFloat;

	if (input.material.metalnessIndex > -1)
	{
		float factor = metallic;
		metallic = textures[input.material.metalnessIndex].Sample(texSampler, input.fragTexCoord).r * factor;
	}

	if (input.material.roughnessIndex > -1)
	{
		float factor = roughness * 2.0f;
		roughness = textures[input.material.roughnessIndex].Sample(texSampler, input.fragTexCoord).r * factor;
	}

	output.gOthers  = float4(0.0f, metallic, roughness, 1.0f);
	output.gDiffuse = float4(albedo, 1.0f);
	output.gNormal  = float4(N, 1.0f);
	//output.gDepth = input.pos.z / input.pos.w;

	return output;
}
