#version 450
layout(binding = 0) uniform sampler2D sceneTexture;
layout(binding = 1) uniform sampler2D blurredTexture;
layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 fragColor;

void main() {
	if(texture(blurredTexture, inUV).x > 0.0f) {
		fragColor = vec4(1.0f, 0.7f, 0.3f, 1.0f);
		return;
	}
	//fragColor = vec4(1.0f, 1.0f, 1.0f, 0.5f);
	discard;
}
