#version 430

layout(rgba32f, binding = 0) uniform image2D inImage;
layout(rgba32f, binding = 1) uniform image2D outImage;

uniform bool downSample;
uniform vec2 inputResolution;
uniform vec2 outputResolution;

uniform int mipLevel = 0;

vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float invGamma = 1.0 / 2.2;
vec3 ToSRGB(vec3 v)   { return PowVec3(v, invGamma); }

float sRGBToLuma(vec3 col)
{
	return dot(col, vec3(0.299f, 0.587f, 0.114f));
}

float KarisAverage(vec3 col)
{
	float luma = sRGBToLuma(ToSRGB(col)) * 0.25f;
	return 1.0f / (1.0f + luma);
}


#define GROUP_SIZE         8
#define GROUP_THREAD_COUNT (GROUP_SIZE * GROUP_SIZE)
#define FILTER_SIZE        3
#define FILTER_RADIUS      (FILTER_SIZE / 2)
#define TILE_SIZE          (GROUP_SIZE + 2 * FILTER_RADIUS)
#define TILE_PIXEL_COUNT   (TILE_SIZE * TILE_SIZE)

int vec2ToIndex(vec2 indices, int numRows) {
    return int(indices.x) + int(indices.y) * numRows;
}

// Function to convert a single index to 2D indices
ivec2 indexToVec2(int index, int numRows) {
    int row = index / numRows;
    int col = index - row * numRows;
    return ivec2(float(row), float(col));
}

shared float sm_r[TILE_PIXEL_COUNT];
shared float sm_g[TILE_PIXEL_COUNT];
shared float sm_b[TILE_PIXEL_COUNT];

void store_lds(int idx, vec4 c)
{
    sm_r[idx] = c.r;
    sm_g[idx] = c.g;
    sm_b[idx] = c.b;
}

//vec4 load_lds(uint idx)
//{
//    return vec4(sm_r[idx], sm_g[idx], sm_b[idx], 1.0);
//}

vec4 load_lds(uint idx)
{
	return vec4(imageLoad(inImage, (indexToVec2(int(idx), int(outputResolution)) / ivec2(inputResolution)) * ivec2(outputResolution) ));
    //return vec4(sm_r[idx], sm_g[idx], sm_b[idx], 1.0);
}


layout (local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE, local_size_z = 1) in;
void main()
{
	vec2 texCoord = gl_GlobalInvocationID.xy;
	vec2 srcTexelSize = 1.0 / inputResolution;
	float x = srcTexelSize.x;
	float y = srcTexelSize.y;

		//imageStore(outImage, ivec2((texCoord)), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	if(downSample)
	{
		vec3 a = imageLoad(inImage, ivec2(texCoord.x - 2*x, texCoord.y + 2*y)).rgb;
		vec3 b = imageLoad(inImage, ivec2(texCoord.x,       texCoord.y + 2*y)).rgb;
		vec3 c = imageLoad(inImage, ivec2(texCoord.x + 2*x, texCoord.y + 2*y)).rgb;
		vec3 d = imageLoad(inImage, ivec2(texCoord.x - 2*x, texCoord.y)).rgb;
		vec3 e = imageLoad(inImage, ivec2(texCoord.x,       texCoord.y)).rgb;
		vec3 f = imageLoad(inImage, ivec2(texCoord.x + 2*x, texCoord.y)).rgb;
		vec3 g = imageLoad(inImage, ivec2(texCoord.x - 2*x, texCoord.y - 2*y)).rgb;
		vec3 h = imageLoad(inImage, ivec2(texCoord.x,       texCoord.y - 2*y)).rgb;
		vec3 i = imageLoad(inImage, ivec2(texCoord.x + 2*x, texCoord.y - 2*y)).rgb;
		vec3 j = imageLoad(inImage, ivec2(texCoord.x - x, texCoord.y + y)).rgb;
		vec3 k = imageLoad(inImage, ivec2(texCoord.x + x, texCoord.y + y)).rgb;
		vec3 l = imageLoad(inImage, ivec2(texCoord.x - x, texCoord.y - y)).rgb;
		vec3 m = imageLoad(inImage, ivec2(texCoord.x + x, texCoord.y - y)).rgb;
		 vec3 downsample;
			vec3 groups[5];
		switch (mipLevel)
		{
		case 0:
		  // We are writing to mip 0, so we need to apply Karis average to each block
		  // of 4 samples to prevent fireflies (very bright subpixels, leads to pulsating
		  // artifacts).
		  groups[0] = (a+b+d+e) * (0.125f/4.0f);
		  groups[1] = (b+c+e+f) * (0.125f/4.0f);
		  groups[2] = (d+e+g+h) * (0.125f/4.0f);
		  groups[3] = (e+f+h+i) * (0.125f/4.0f);
		  groups[4] = (j+k+l+m) * (0.5f/4.0f);
		  groups[0] *= KarisAverage(groups[0]);
		  groups[1] *= KarisAverage(groups[1]);
		  groups[2] *= KarisAverage(groups[2]);
		  groups[3] *= KarisAverage(groups[3]);
		  groups[4] *= KarisAverage(groups[4]);
		  downsample = groups[0]+groups[1]+groups[2]+groups[3]+groups[4];
		  downsample = max(downsample, 0.0001f);
		  break;
		default:
		  downsample = e*0.125;                // ok
		  downsample += (a+c+g+i)*0.03125;     // ok
		  downsample += (b+d+f+h)*0.0625;      // ok
		  downsample += (j+k+l+m)*0.125;       // ok
		  break;
		}

		//imageStore(outImage, ivec2((texCoord)), vec4(1.0f, 1.0f, 1.0f, 1.0f));
				imageStore(outImage, ivec2((texCoord / inputResolution) * outputResolution), vec4(downsample, 1.0f));
	}
	else
	{
		srcTexelSize = 1.0f / inputResolution;
		ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
		vec2  base_index   = ivec2(gl_WorkGroupID) * GROUP_SIZE - FILTER_RADIUS;

		// The first (TILE_PIXEL_COUNT - GROUP_THREAD_COUNT) threads load at most 2 texel values
		for (int i = int(gl_LocalInvocationIndex); i < TILE_PIXEL_COUNT; i += GROUP_THREAD_COUNT)
		{
		    vec2 uv        = (base_index + 0.5) * srcTexelSize;
		    vec2 uv_offset = vec2(i % TILE_SIZE, i / TILE_SIZE) * srcTexelSize;

		    vec4 color = imageLoad(inImage, ivec2(uv + uv_offset));
		    store_lds(i, color);
		}

		memoryBarrierShared();
		barrier();

		// center texel
		uint sm_idx = (gl_LocalInvocationID.x + FILTER_RADIUS) + (gl_LocalInvocationID.y + FILTER_RADIUS) * TILE_SIZE;

		// Based on [Jimenez14] http://goo.gl/eomGso
		vec4 s;
		s =  load_lds(sm_idx - TILE_SIZE - 1);
		s += load_lds(sm_idx - TILE_SIZE    ) * 2.0;
		s += load_lds(sm_idx - TILE_SIZE + 1);

		s += load_lds(sm_idx - 1) * 2.0;
		s += load_lds(sm_idx    ) * 4.0;
		s += load_lds(sm_idx + 1) * 2.0;

		s += load_lds(sm_idx + TILE_SIZE - 1);
		s += load_lds(sm_idx + TILE_SIZE    ) * 2.0;
		s += load_lds(sm_idx + TILE_SIZE + 1);

		vec4 bloom = s * (1.0 / 16.0);

		vec4 out_pixel = imageLoad(outImage, ivec2(texCoord));
		float u_bloom_intensity = 1.0f;
		out_pixel += bloom * u_bloom_intensity;

		imageStore(outImage, ivec2(texCoord), bloom);

	}
}
