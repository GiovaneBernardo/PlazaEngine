#version 460 core
layout(push_constant) uniform PushConstants{
    mat4 projection;
    mat4 view;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 instanceMatrix[4];

void main() {
    mat4 model = mat4(instanceMatrix[0], instanceMatrix[1], instanceMatrix[2], instanceMatrix[3]);
    vec4 finalModel = model *  vec4(inPosition,1.0f);
    gl_Position = pushConstants.projection * pushConstants.view * finalModel;
}
