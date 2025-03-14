#version 460

layout(binding = 0)			 uniform sampler2D u_input_texture;
layout(rgba32f, binding = 0) uniform image2D   u_output_image;

layout(binding = 1)			 uniform sampler2D u_dirt_texture;

float mThreshold = 1.5f;
float mKnee = 0.1f;
vec4 u_threshold = vec4(mThreshold, mThreshold - mKnee, 2.0f * mKnee, 0.25f * mKnee);
uniform vec2  u_texel_size;
uniform int   u_mip_level;
uniform float u_bloom_intensity;
uniform float u_dirt_intensity;

#define GROUP_SIZE         8
#define GROUP_THREAD_COUNT (GROUP_SIZE * GROUP_SIZE)
#define FILTER_SIZE        3
#define FILTER_RADIUS      (FILTER_SIZE / 2)
#define TILE_SIZE          (GROUP_SIZE + 2 * FILTER_RADIUS)
#define TILE_PIXEL_COUNT   (TILE_SIZE * TILE_SIZE)

shared float sm_r[TILE_PIXEL_COUNT];
shared float sm_g[TILE_PIXEL_COUNT];
shared float sm_b[TILE_PIXEL_COUNT];

void store_lds(int idx, vec4 c)
{
    sm_r[idx] = c.r;
    sm_g[idx] = c.g;
    sm_b[idx] = c.b;
}

vec4 load_lds(uint idx)
{
    return vec4(sm_r[idx], sm_g[idx], sm_b[idx], 1.0);
}


const float epsilon = 1.0e-4;

// Curve = (threshold - knee, knee * 2.0, knee * 0.25)
vec4 quadratic_threshold(vec4 color, float threshold, vec3 curve)
{
	// Pixel brightness
    float br = max(color.r, max(color.g, color.b));

    // Under-threshold part: quadratic curve
    float rq = clamp(br - curve.x, 0.0, curve.y);
    rq = curve.z * rq * rq;

    // Combine and apply the brightness response curve.
    color *= max(rq, br - threshold) / max(br, epsilon);

    return color;
}

layout(local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE) in;
void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    vec2  base_index   = ivec2(gl_WorkGroupID) * GROUP_SIZE - FILTER_RADIUS;

    // The first (TILE_PIXEL_COUNT - GROUP_THREAD_COUNT) threads load at most 2 texel values
    for (int i = int(gl_LocalInvocationIndex); i < TILE_PIXEL_COUNT; i += GROUP_THREAD_COUNT)
    {
        vec2 uv        = (base_index + 0.5) * u_texel_size;
        vec2 uv_offset = vec2(i % TILE_SIZE, i / TILE_SIZE) * u_texel_size;

        vec4 color = textureLod(u_input_texture, (uv + uv_offset), u_mip_level);
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

	vec4 out_pixel = imageLoad(u_output_image, pixel_coords);
    out_pixel += bloom * 1.3f;

    // out_pixel = quadratic_threshold(out_pixel, u_threshold.x, u_threshold.yzw);

	imageStore(u_output_image, pixel_coords, out_pixel);
}
