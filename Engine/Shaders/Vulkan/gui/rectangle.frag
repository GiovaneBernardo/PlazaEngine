#version 450 core

layout(location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2D samplerTexture;
layout (binding = 1) uniform sampler2D fontTexture;

layout(push_constant) uniform PushConstants{
    mat4 matrix;
} pushConstants;

void main(void)
{
    // Calculate normalized coordinates
   // vec2 uv = gl_FragCoord.xy / vec2(1820, 720); // Adjust for your window size
   // uv = uv * 2.0 - 1.0; // Convert to range [-1, 1]

    // Draw a simple white plane
    outColor = vec4(0.3, 0.3, 0.3, 0.5); // White color
}
