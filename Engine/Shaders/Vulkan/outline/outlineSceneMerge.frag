#version 450
layout(binding = 0) uniform sampler2D sceneTexture;
layout(binding = 1) uniform sampler2D blurredTexture;
layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = vec4(texture(sceneTexture, inUV).xyz + texture(blurredTexture, inUV).xyz, 1.0f);
}
