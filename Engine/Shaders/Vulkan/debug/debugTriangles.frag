#version 450 core

layout(location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2D samplerTexture;

layout(push_constant) uniform PushConstants{
    mat4 matrix;
} pushConstants;

void main(void)
{
    outColor = vec4(0.3, 0.3, 0.3, 0.5);
}
