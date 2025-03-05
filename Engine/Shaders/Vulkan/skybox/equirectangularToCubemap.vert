#version 450

layout(location = 0) out vec3 WorldPos;


layout(push_constant) uniform PushConstants{
    mat4 mvp;
    bool first;
    float deltaPhi;
	float deltaTheta;
    float roughness;
    uint numSamples;
} pushConstants;

vec3 positions[36] = vec3[](
    // Front face
    vec3(-1.0, -1.0,  1.0), vec3( 1.0, -1.0,  1.0), vec3( 1.0,  1.0,  1.0),
    vec3(-1.0, -1.0,  1.0), vec3( 1.0,  1.0,  1.0), vec3(-1.0,  1.0,  1.0),

    // Back face
    vec3(-1.0, -1.0, -1.0), vec3(-1.0,  1.0, -1.0), vec3( 1.0,  1.0, -1.0),
    vec3(-1.0, -1.0, -1.0), vec3( 1.0,  1.0, -1.0), vec3( 1.0, -1.0, -1.0),

    // Top face
    vec3(-1.0,  1.0, -1.0), vec3(-1.0,  1.0,  1.0), vec3( 1.0,  1.0,  1.0),
    vec3(-1.0,  1.0, -1.0), vec3( 1.0,  1.0,  1.0), vec3( 1.0,  1.0, -1.0),

    // Bottom face
    vec3(-1.0, -1.0, -1.0), vec3( 1.0, -1.0, -1.0), vec3( 1.0, -1.0,  1.0),
    vec3(-1.0, -1.0, -1.0), vec3( 1.0, -1.0,  1.0), vec3(-1.0, -1.0,  1.0),

    // Right face
    vec3( 1.0, -1.0, -1.0), vec3( 1.0,  1.0, -1.0), vec3( 1.0,  1.0,  1.0),
    vec3( 1.0, -1.0, -1.0), vec3( 1.0,  1.0,  1.0), vec3( 1.0, -1.0,  1.0),

    // Left face
    vec3(-1.0, -1.0, -1.0), vec3(-1.0, -1.0,  1.0), vec3(-1.0,  1.0,  1.0),
    vec3(-1.0, -1.0, -1.0), vec3(-1.0,  1.0,  1.0), vec3(-1.0,  1.0, -1.0)
);

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
    vec3 pos = positions[gl_VertexIndex];
    WorldPos = pos;
    gl_Position = pushConstants.mvp * vec4(pos, 1.0);
}
