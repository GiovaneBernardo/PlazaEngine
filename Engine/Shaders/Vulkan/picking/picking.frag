#version 450
layout(location = 0) out uvec4 fragColor;

float hash(uint x_low, uint x_high) {
    uint combined = x_low ^ x_high; // XOR the two uint32_t values
    uint hash_value = (combined * 2654435761u); // A simple prime number multiplier

    return float(hash_value & 0xFFFFFF) / float(0xFFFFFF); // Normalize to [0, 1]
}

layout(push_constant) uniform PushConstants{
    mat4 projection;
    mat4 view;
    mat4 model;
	uint uuid1;
	uint uuid2;
} pushConstants;

void main() {
    fragColor = uvec4(pushConstants.uuid1, pushConstants.uuid2, 1.0f, 1.0f);
    //fragColor = vec4(hash(pushConstants.uuid1, pushConstants.uuid2), 1.0f, 1.0f, 1.0f);
    //fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
