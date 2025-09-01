#version 450 core

layout (location = 0) in vec2 inUV;

layout (binding = 0) uniform sampler2D samplerTexture;
layout (binding = 1) uniform sampler2D fontTexture;

layout (location = 0) out vec4 outFragColor;

layout(push_constant) uniform PushConstants{
    mat4 matrix;
} pushConstants;

void main(void)
{
	float color = texture(fontTexture, vec2(inUV.x, inUV.y)).r;
	outFragColor = vec4(color);
}
