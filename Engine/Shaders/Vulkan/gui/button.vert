#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec4 instanceMatrix[4];
layout(location = 8) in uint vertexMaterialIndex;

layout (location = 0) out vec2 outUV;

layout(push_constant) uniform PushConstants{
    mat4 matrix;
} pushConstants;

struct RectangleData {
    mat4 transform;
    vec4 color;
    int textureIndex;
};

out gl_PerVertex
{
	vec4 gl_Position;
};

void main(void)
{
	mat4 model = mat4(instanceMatrix[0], instanceMatrix[1], instanceMatrix[2], instanceMatrix[3]);

    // Perform multiplication
    gl_Position = pushConstants.matrix * model * vec4(inPosition.xy, 0.0f, 1.0f);
    outUV = inTexCoord;
}
