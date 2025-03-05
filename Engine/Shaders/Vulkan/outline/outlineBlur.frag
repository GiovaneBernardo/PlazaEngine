#version 450
layout(binding = 0) uniform sampler2D outlineTexture;
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 fragColor;

const float blurSizeX = 1.0f / 1820.0f;
const float blurSizeY = 1.0f / 720.0f;

void main() {
    vec3 col = vec3(0.0f);
    int kernelRadius = 3;

    for (int y = -kernelRadius; y <= kernelRadius; y++) {
        for (int x = -kernelRadius; x <= kernelRadius; x++) {
            col += texture(outlineTexture, vec2(inUV.x + x * blurSizeX, inUV.y + y * blurSizeY)).xyz;
        }
    }

    int totalSamples = (kernelRadius * 2 + 1) * (kernelRadius * 2 + 1);
    fragColor = vec4(col.xyz / float(totalSamples), 1.0f);
}
