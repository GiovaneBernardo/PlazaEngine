#version 450 core

layout(location = 0) in vec3 inPosition;

layout (location = 0) out vec2 outUV;

layout(push_constant) uniform PushConstants{
    mat4 viewMatrix;
} pushConstants;

struct DebugLine {
    vec3 start;
    vec3 end;
    vec3 thickness;
    vec4 color;
};

struct DebugBox {
    vec3 position;
    vec3 rotation;
    vec3 scale;
    vec4 color;
};

// 3D
struct DebugSphere {
    vec3 position;
    vec3 rotation;
    vec3 scale;
    vec4 color;
    float radius;
};

struct DebugRectangle {
    vec3 position;
    vec3 rotation;
    vec3 scale;
    vec4 color;
    vec3 corners[8];
};

void main(void)
{
    mat4 model = mat4(1.0f);//GetTransform();
    gl_Position = pushConstants.viewMatrix * model * vec4(inPosition.xy, 0.0f, 1.0f);
}
