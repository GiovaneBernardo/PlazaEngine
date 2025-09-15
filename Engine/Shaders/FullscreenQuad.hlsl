struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

VSOutput mainVS(uint vertexID : SV_VertexID)
{
	VSOutput output;
	float2 pos = float2((vertexID << 1) & 2, vertexID & 2);
	output.pos = float4(pos * float2(2.0, 2.0) - 1.0, 0.0, 1.0);
	output.uv  = pos;
	return output;
}