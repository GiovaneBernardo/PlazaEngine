#define TILE_SIZE 8
#define NUM_SAMPLES 4
#define MAX_STEPS 16
#define STEP_SIZE 0.05
#define MAX_DISTANCE 50.0

#pragma pack_matrix(column_major)

// ------------------------------------------------------------
// Resources
// ------------------------------------------------------------

Texture2D<float4> GDiffuse : register(t0);
Texture2D<float4> GNormal : register(t1);
Texture2D<float> GDepth : register(t2);
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

RWTexture2D<float4> SSGIOutput : register(u5);

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

// Simple hash for randomness
float hash(float2 p)
{
	float3 p3 = frac(float3(p.xyx) * 0.1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return frac((p3.x + p3.y) * p3.z);
}

float2 hash2(float2 p)
{
	return float2(hash(p), hash(p + float2(127.1, 311.7)));
}

// Reconstruct view-space position from UV and depth
float3 ReconstructViewPosition(float2 uv, float depth)
{
	// Flip Y for Vulkan
	float2 uv2 = float2(uv.x, 1.0f - uv.y);
	float4 clip = float4(uv2 * 2.0f - 1.0f, depth, 1.0f);
	float4 viewPos = mul(invProjection, clip);
	return viewPos.xyz / viewPos.w;
}

// Reconstruct world-space position
float3 ReconstructWorldPosition(float2 uv, float depth)
{
	float3 viewPos = ReconstructViewPosition(uv, depth);
	float4 worldPos = mul(invView, float4(viewPos, 1.0f));
	return worldPos.xyz;
}

// Project world position to screen UV
float2 ProjectToScreen(float3 worldPos, out float projDepth)
{
	float4 viewPos = mul(view, float4(worldPos, 1.0f));
	float4 clipPos = mul(projection, viewPos);

	if (clipPos.w <= 0.0)
	{
		projDepth = -1.0;
		return float2(-1, -1);
	}

	float3 ndc = clipPos.xyz / clipPos.w;
	projDepth = ndc.z;

	// NDC to UV (Vulkan Y is top-down)
	float2 uv = float2(ndc.x * 0.5 + 0.5, 0.5 - ndc.y * 0.5);
	return uv;
}

// Get random direction in hemisphere around normal
float3 RandomHemisphereDirection(float3 normal, float2 seed)
{
	float2 r = hash2(seed);

	// Cosine-weighted hemisphere sampling
	float phi = 2.0 * 3.14159265 * r.x;
	float cosTheta = sqrt(1.0 - r.y);
	float sinTheta = sqrt(r.y);

	// Create tangent space
	float3 up = abs(normal.y) < 0.999 ? float3(0, 1, 0) : float3(1, 0, 0);
	float3 tangent = normalize(cross(up, normal));
	float3 bitangent = cross(normal, tangent);

	// Convert to world space
	float3 dir = tangent * (cos(phi) * sinTheta) +
	             bitangent * (sin(phi) * sinTheta) +
	             normal * cosTheta;

	return normalize(dir);
}

// ------------------------------------------------------------
// Main Compute Shader
// ------------------------------------------------------------

[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void mainCS(
    uint3 dispatchID : SV_DispatchThreadID,
    uint3 groupID : SV_GroupID,
    uint localIndex : SV_GroupIndex)
{
	int2 screenSizee = float2(1920, 1080); // ----qweqeqweeqweqw------------------qweqeqweeqweqw----------------------------------------------- THE ZOOMED IN GI WAS FIXED BY MANUALLY SETTING THE RESOLUTION HERE, THIS IS A WORKAROUND BUT THE FIX IS RELATED TO IT
	uint2 pixel = dispatchID.xy;

	// Bounds check
	if (pixel.x >= (uint)screenSizee.x || pixel.y >= (uint)screenSizee.y)
		return;

	float2 uv = (float2(pixel) + 0.5) / screenSizee;

	// Sample G-buffer
	float depth = GDepth.SampleLevel(texSampler, uv, 0);

	// Skip sky pixels (depth = 1.0 for standard depth buffer)
	if (depth > 0.9999)
	{
		SSGIOutput[pixel] = float4(0, 0, 0, 0);
		return;
	}

	float3 normal = GNormal.SampleLevel(texSampler, uv, 0).xyz;
	normal = normalize(normal * 2.0 - 1.0); // Assuming normal is stored as [0,1]

	float3 worldPos = ReconstructWorldPosition(uv, depth);
	float3 viewPos = ReconstructViewPosition(uv, depth);

	// Transform normal to world space if it's in view space
	float3 worldNormal = normalize(mul((float3x3)invView, normal));

	float3 indirect = float3(0, 0, 0);
	float hitCount = 0;

	// Cast multiple rays
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		// Random direction in hemisphere
		float2 seed = float2(pixel) + float2(i * 127.1, i * 311.7);
		float3 rayDir = RandomHemisphereDirection(worldNormal, seed);

		// Ray march in screen space
		float3 rayPos = worldPos + worldNormal * 0.1; // Offset to avoid self-intersection

		bool hit = false;
		float3 hitColor = float3(0, 0, 0);

		for (int step = 1; step <= MAX_STEPS; step++)
		{
			// Step along ray in world space
			float stepDist = STEP_SIZE * step * step; // Exponential steps
			rayPos = worldPos + rayDir * stepDist;

			// Check max distance
			if (stepDist > MAX_DISTANCE)
				break;

			// Project to screen
			float projDepth;
			float2 sampleUV = ProjectToScreen(rayPos, projDepth);

			// Check if outside screen
			if (sampleUV.x < 0 || sampleUV.x > 1 || sampleUV.y < 0 || sampleUV.y > 1)
				break;

			if (projDepth < 0)
				break;

			// Sample scene depth at this UV
			float sceneDepth = GDepth.SampleLevel(texSampler, sampleUV, 0);

			// Check for intersection (ray is behind scene geometry)
			// Using a thickness threshold to avoid thin object issues
			float depthDiff = projDepth - sceneDepth;

			if (depthDiff > 0.0001 && depthDiff < 0.01)
			{
				// Hit! Sample the color
				hitColor = GDiffuse.SampleLevel(texSampler, sampleUV, 0).rgb;
				hit = true;
				break;
			}
		}

		if (hit)
		{
			// Weight by cosine (already done in hemisphere sampling)
			indirect += hitColor;
			hitCount += 1.0;
		}
	}

	// Average indirect lighting
	if (hitCount > 0)
	{
		indirect /= hitCount;
	}

	// Apply some falloff/intensity
	indirect *= 0.5;

	SSGIOutput[pixel] = float4(indirect, 1.0);
}
