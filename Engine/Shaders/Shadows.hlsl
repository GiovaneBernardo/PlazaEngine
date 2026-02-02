#pragma extension SPV_KHR_multiview

#define MAX_BONES 1000
#define MAX_BONE_INFLUENCE 4

struct VSInput {
    float3 aPos              : TEXCOORD0;
    float4 instanceMatrix0   : TEXCOORD1;
    float4 instanceMatrix1   : TEXCOORD2;
    float4 instanceMatrix2   : TEXCOORD3;
    float4 instanceMatrix3   : TEXCOORD4;
    // int4 boneIds           : TEXCOORD9;
    // float4 weights         : TEXCOORD10;
};

// Output to pixel shader
struct VSOutput {
    float2 uv     : TEXCOORD0;
    float4 pos    : SV_POSITION;
};

// Uniform Buffer Object
[[vk::binding(0, 0)]]
cbuffer ShadowsUBO
{
    column_major float4x4 lightSpaceMatrices[32];
};

// StructuredBuffer for bone matrices
[[vk::binding(1, 0)]]
StructuredBuffer<float4x4> BoneMatrices;

// Main VS
VSOutput mainVS(VSInput input, uint viewID : SV_ViewID)
{
	VSOutput output;
	float4x4 model = float4x4(
		float4(input.instanceMatrix0.x, input.instanceMatrix1.x, input.instanceMatrix2.x, input.instanceMatrix3.x),
		float4(input.instanceMatrix0.y, input.instanceMatrix1.y, input.instanceMatrix2.y, input.instanceMatrix3.y),
		float4(input.instanceMatrix0.z, input.instanceMatrix1.z, input.instanceMatrix2.z, input.instanceMatrix3.z),
		float4(input.instanceMatrix0.w, input.instanceMatrix1.w, input.instanceMatrix2.w, input.instanceMatrix3.w)
	);
    
	float4 totalPosition = float4(0.0f, 0.0f, 0.0f, 0.0f);
	bool allNegative = true;
    
	if (allNegative)
		totalPosition = float4(input.aPos, 1.0f);
    
	float4 worldPos = mul(model, totalPosition);
	output.pos = mul(lightSpaceMatrices[viewID], worldPos); // Fixed: use viewID!
    
    // Manually flip Y for Vulkan
	//output.pos.y = -output.pos.y;
    
	output.uv = input.aPos.xy;
	return output;
}

struct PSInput {
    float2 uv : TEXCOORD0;
};

//struct PSOutput {
//    float4 ShadowsDepthMap    : SV_Target0;
//};

void mainPS(PSInput input, uint viewID : SV_ViewID)
{
  //  PSOutput output;
   // output.ShadowsDepthMap   = float4(1, 0, 0, 1);
   // return output;
}
