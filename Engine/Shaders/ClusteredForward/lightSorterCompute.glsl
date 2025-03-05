#version 430 core
struct LightStruct {
    vec3 color;
    float radius;
    vec3 position;
    float intensity;
};

layout(std430, binding = 0) buffer LightsArray {
    LightStruct lights[];
};

struct Cluster{
    int[256] lightsIndex;
    int lightsCount;
    vec3 minBounds;
    vec3 maxBounds;
};

layout(std430, binding = 1) buffer ClusterBuffer {
    Cluster clusters[];
};
layout (location = 3) uniform mat4 view;
layout (location = 4) uniform mat4 projection;
layout (location = 5) uniform int lightCount;
layout (location = 6) uniform bool first;

vec2 screenSize = vec2(1820, 720);

vec2 clusterSize = vec2(32, 32);

layout(std430, binding = 6) buffer OutputBuffer {
    vec3[] result;
};

layout (std430, binding = 8) buffer DepthTileBuffer {
    vec2 tileDepthStats[];
};

uniform sampler2D depthMap;

const int tileSizeX = 32;
const int tileSizeY = 32;

shared uint minDepthInt;
shared uint maxDepthInt;
shared uint visibleLightCount;
shared vec4 frustumPlanes[6];

shared int visibleLightIndices[1024];

shared mat4 viewProjection;
const int TILE_SIZE = 32;

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
    ivec2 location = ivec2(gl_GlobalInvocationID.xy);
	ivec2 itemID = ivec2(gl_LocalInvocationID.xy);
	ivec2 tileID = ivec2(gl_WorkGroupID.xy);
	ivec2 tileNumber = ivec2(gl_NumWorkGroups.xy);
	uint index = tileID.y * tileNumber.x + tileID.x;
    vec2 clusterCount = ceil(screenSize / clusterSize);
    int totalClusterCount = int(ceil(clusterCount.x * clusterCount.y));
    int x = int(gl_WorkGroupID.x);
    int y = int(gl_WorkGroupID.y);
    int clusterIndex = int(y * clusterCount.x + x);

    if(lightCount <= 0)
    {
        if (gl_LocalInvocationIndex == 0) {
            clusters[index].lightsCount = 0;
            for(int i = 0; i < lightCount; i++)
            {
                clusters[index].lightsIndex[i] = -1;
            }
        }
        return;
    }

 vec4 frustumPlanes[6];


	// Initialize shared global values for depth and light count
	if (gl_LocalInvocationIndex == 0) {
		minDepthInt = 0xFFFFFFFF;
		maxDepthInt = 0;
		visibleLightCount = 0;
		viewProjection = projection * view;
	}

	barrier();

	// Step 1: Calculate the minimum and maximum depth values (from the depth buffer) for this group's tile
	float maxDepth, minDepth;
	vec2 text = vec2(location) / ivec2(screenSize.x, screenSize.y);
	float depth = texture(depthMap, text).r;
	// Linearize the depth value from depth buffer (must do this because we created it using projection)
	//depth = projection * depth;//(0.5 * projection[3][2]) / (depth + 0.5 * projection[2][2] );

	// Convert depth to uint so we can do atomic min and max comparisons between the threads
	uint depthInt = floatBitsToUint(depth);
	atomicMin(minDepthInt, depthInt);
	atomicMax(maxDepthInt, depthInt);

    barrier();

    if (gl_LocalInvocationIndex == 0) {
        // Convert the min and max across the entire tile back to float
        minDepth = uintBitsToFloat(minDepthInt);
        maxDepth = uintBitsToFloat(maxDepthInt);

		// Steps based on tile sale
		vec2 negativeStep = (2.0 * vec2(tileID)) / vec2(tileNumber);
		vec2 positiveStep = (2.0 * vec2(tileID + ivec2(1, 1))) / vec2(tileNumber);

		// Set up starting values for planes using steps and min and max z values
		frustumPlanes[0] = vec4(1.0, 0.0, 0.0, 1.0 - negativeStep.x); // Left
		frustumPlanes[1] = vec4(-1.0, 0.0, 0.0, -1.0 + positiveStep.x); // Right
		frustumPlanes[2] = vec4(0.0, 1.0, 0.0, 1.0 - negativeStep.y); // Bottom
		frustumPlanes[3] = vec4(0.0, -1.0, 0.0, -1.0 + positiveStep.y); // Top
		frustumPlanes[4] = vec4(0.0, 0.0, -1.0, minDepth); // Near
		frustumPlanes[5] = vec4(0.0, 0.0, 1.0, maxDepth); // Far

		// Transform the first four planes
		for (uint i = 0; i < 4; i++) {
			frustumPlanes[i] *= viewProjection;
			frustumPlanes[i] /= length(frustumPlanes[i].xyz);
		}

		// Transform the depth planes
		frustumPlanes[4] *= view;
		frustumPlanes[4] /= length(frustumPlanes[4].xyz);
		frustumPlanes[5] *= view;
		frustumPlanes[5] /= length(frustumPlanes[5].xyz);
	}
    barrier();



//    int lightCount = 256;

    tileDepthStats[index] = vec2(minDepth, maxDepth);
    if (gl_LocalInvocationIndex == 0) {
        clusters[clusterIndex].lightsCount = 0;
        for(int i = 0; i < lightCount + 1; i++)
        {
            clusters[index].lightsIndex[i] = -1;
        }
    }
    barrier();
	uint threadCount = TILE_SIZE * TILE_SIZE;
    uint passCount = (lightCount + threadCount - 1) / threadCount;
    if (gl_LocalInvocationIndex == 0) {
        for (uint i = 0; i < lightCount; i++)
        {
            // Get the lightIndex to test for this thread / pass. If the index is >= light count, then this thread can stop testing lights
            uint lightIndex = i;
            if (lightIndex >= lightCount) {
            	break;
            }

            vec4 position = vec4(lights[lightIndex].position.xyz, 1.0f);
            float radius = lights[lightIndex].radius;

            // We check if the light exists in our frustum
            float distance = 0.0;
            for (uint j = 0; j < 6; j++) {
            	distance = dot(position, frustumPlanes[j]) + radius; // lights[lightIndex].radius);

            	// If one of the tests fails, then there is no intersection
            	if (distance <= 0.0) {
            		break;
            	}
            }

            // If greater than zero, then it is a visible light
            if (distance > 0.0) {
            	// Add index to the shared array of visible indices
                            //int lightIndex = int(clusters[index].lightsCount);

            	uint offset = atomicAdd(visibleLightCount, 1);
            	clusters[index].lightsIndex[offset] = int(lightIndex);

            }
        }
    }
}
