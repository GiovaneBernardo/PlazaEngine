#version 450
layout(location = 0) in vec3 fragTexCoord;
layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform samplerCube skyboxSampler;

void main() {
    fragColor = texture(skyboxSampler, fragTexCoord);
}
