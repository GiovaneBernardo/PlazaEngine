#pragma extension SPV_EXT_descriptor_indexing

struct UniformBufferObject
{
	column_major float4x4 projection;
	column_major float4x4 view;
};

cbuffer UBO : register(b0)
{
	UniformBufferObject ubo;
}

struct DebugLine
{
	float3 start;
	float pad0;
	float3 end;
	float thickness;
	float4 color;
};

[[vk::binding(1, 0)]]
StructuredBuffer<DebugLine> DebugLinesSSBO : register(t1);

[[vk::binding(2, 0)]]
StructuredBuffer<DebugLine> DebugBoxesSSBO : register(t2);

[[vk::binding(3, 0)]]
StructuredBuffer<DebugLine> DebugSpheresSSBO : register(t3);

struct VSOutput
{
	float4 color : TEXCOORD0;
	float4 pos : SV_POSITION;
};

// Main VS
VSOutput mainVS(
    uint vertexID : SV_VertexID,
    uint instanceID : SV_InstanceID
)
{
	VSOutput output;

	DebugLine dbgLine = DebugLinesSSBO[instanceID];
	
    // Each instance emits exactly 2 vertices
	float3 worldPos = (vertexID == 0)
        ? dbgLine.start
        : dbgLine.end;

	float4 viewPos = mul(ubo.view, float4(worldPos, 1.0));
	output.pos = mul(ubo.projection, viewPos);
	output.pos.y *= -1.0f;
	output.color = dbgLine.color;
	return output;
}

struct PSInput
{
	float4 color : TEXCOORD0;
	float4 pos : SV_POSITION;
};

struct PSOutput
{
	float4 fragColor : SV_Target0;
};

PSOutput mainPS(PSInput input)
{
	PSOutput output;
	output.fragColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	return output;
}
