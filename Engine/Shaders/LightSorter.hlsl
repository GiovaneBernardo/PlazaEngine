#define TILE_SIZE 32
#define MAX_LIGHTS_PER_TILE 256

#pragma pack_matrix(column_major)

// ------------------------------------------------------------
// Structures
// ------------------------------------------------------------

struct LightStruct
{
	float3 color;
	float radius;
	float3 position; // world space
	float intensity;
	float cutoff;
	float minRadius;
	float2 _pad;
};

struct Cluster
{
	int lightsIndex[MAX_LIGHTS_PER_TILE];
	int lightsCount;
	float3 minBounds;
	float3 maxBounds;
};

// ------------------------------------------------------------
// Resources
// ------------------------------------------------------------

StructuredBuffer<LightStruct> LightsArray : register(t0);
RWStructuredBuffer<Cluster> ClusterBuffer : register(u1);

Texture2D<float> depthMap : register(t2);
SamplerState texSampler : register(s3);

cbuffer CameraData : register(b4)
{
	float4x4 view;
	float4x4 projection;
	float4x4 invProjection;
	float4x4 invView;
	int lightCount;
	float3 _pad0;
	float2 screenSize;
	float2 tileSize;
};

RWStructuredBuffer<float2> DepthTileBuffer : register(u8);

// ------------------------------------------------------------
// Groupshared
// ------------------------------------------------------------

groupshared uint sNearDepthBits; // max (reversed-Z)
groupshared uint sFarDepthBits; // min (reversed-Z)

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

float3 ReconstructViewPosition(float2 uv, float depth)
{
	float2 uv2 = float2(uv.x, 1.0f - uv.y);
	float4 clip = float4(uv2 * 2.0f - 1.0f, depth, 1.0f);
	float4 viewPos = mul(invProjection, clip);
	return viewPos.xyz / viewPos.w;
}

// ------------------------------------------------------------
// Compute Shader
// ------------------------------------------------------------

[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void mainCS(
    uint3 dispatchID : SV_DispatchThreadID,
    uint3 groupID : SV_GroupID,
    uint localIndex : SV_GroupIndex)
{
	uint2 pixel = dispatchID.xy;

	if (pixel.x >= (uint) screenSize.x ||
        pixel.y >= (uint) screenSize.y)
		return;

	uint tilesX = (uint) ((screenSize.x + tileSize.x - 1) / tileSize.x);
	uint tileIndex = groupID.y * tilesX + groupID.x;

    // --------------------------------------------------------
    // Init
    // --------------------------------------------------------

	if (localIndex == 0)
	{
		sNearDepthBits = 0; // reversed-Z max
		sFarDepthBits = 0xFFFFFFFF; // reversed-Z min
	}

	GroupMemoryBarrierWithGroupSync();

    // --------------------------------------------------------
    // Depth reduction
    // --------------------------------------------------------

	float2 uv = (float2(pixel) + 0.5f) / screenSize;
	float depth = depthMap.SampleLevel(texSampler, uv, 0);

	uint d = asuint(depth);
	InterlockedMax(sNearDepthBits, d);
	InterlockedMin(sFarDepthBits, d);

	GroupMemoryBarrierWithGroupSync();

    // --------------------------------------------------------
    // Tile setup (single thread)
    // --------------------------------------------------------

	if (localIndex == 0)
	{
		float nearDepth = asfloat(sNearDepthBits);
		float farDepth = asfloat(sFarDepthBits);

		DepthTileBuffer[tileIndex] = float2(nearDepth, farDepth);

        // Reversed-Z validity test
		if (nearDepth <= farDepth)
		{
			ClusterBuffer[tileIndex].lightsCount = 0;
			return;
		}

        // Reconstruct tile frustum in VIEW SPACE
		float2 tileMin = (groupID.xy * tileSize) / screenSize;
		float2 tileMax = ((groupID.xy + 1) * tileSize) / screenSize;

		float3 v00 = ReconstructViewPosition(tileMin, nearDepth);
		float3 v11 = ReconstructViewPosition(tileMax, farDepth);

		float3 minV = min(v00, v11);
		float3 maxV = max(v00, v11);

		ClusterBuffer[tileIndex].minBounds = minV;
		ClusterBuffer[tileIndex].maxBounds = maxV;

        // ----------------------------------------------------
        // Light culling (AABB vs sphere in view space)
        // ----------------------------------------------------

		int count = 0;

		for (int i = 0; i < lightCount && count < MAX_LIGHTS_PER_TILE; i++)
		{
			float3 lightPosView = mul(view, float4(LightsArray[i].position, 1.0f)).xyz;
			float r = LightsArray[i].radius;
			r *= 2.0f;
			float3 closest =
                clamp(lightPosView, minV, maxV);

			float distSq = dot(lightPosView - closest,
                               lightPosView - closest);

			if (distSq <= r * r)
			{
				ClusterBuffer[tileIndex].lightsIndex[count++] = i;
			}
		}

		ClusterBuffer[tileIndex].lightsCount = count;
	}
}
