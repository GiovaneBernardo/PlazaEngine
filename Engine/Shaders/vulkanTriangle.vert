#version 460 core
layout(binding = 0) uniform UniformBufferObject {
    mat4 projection;
    mat4 view;
    mat4 model;
    int cascadeCount;
    float farPlane;
    float nearPlane;
    vec4 lightDirection;
    vec4 viewPos;
    mat4[16] lightSpaceMatrices;
    vec4[16] cascadePlaneDistances;
    vec4 sunColor;
    bool showCascadeLevels;
} ubo;
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in vec4 instanceMatrix[4];

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out mat4 model;

layout(location = 18) in uint mat;
layout(binding = 20) uniform sampler2D textures[];

layout(location = 10) out uint materialIndex;
layout(location = 11) out vec4 FragPos;
layout(location = 12) out vec4 Normal;
layout(location = 13) out vec2 TexCoords;
layout(location = 14) out vec4 TangentLightPos;
layout(location = 15) out vec4 TangentViewPos;
layout(location = 16) out vec4 TangentFragPos;
layout(location = 17) out vec4 worldPos;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
    model = mat4(instanceMatrix[0], instanceMatrix[1], instanceMatrix[2], instanceMatrix[3]);

    mat4 aInstanceMatrix = model;
    fragTexCoord = vec2(0.0f, 1.0f) - inTexCoord;

    mat4 finalInstanceMatrix = model;
    vec4 finalModel = finalInstanceMatrix * vec4(inPosition, 1.0);
    FragPos = vec4(model * vec4(inPosition, 1.0));
    //vs_out.Normal = transpose(inverse(mat3(aInstanceMatrix))) * aNormal;
    TexCoords = vec2(1.0f, 1.0f) -  inTexCoord;

    //if(usingNormal){
    //    mat3 normalMatrix = transpose(inverse(mat3(finalInstanceMatrix)));
    //    vec3 T = normalize(normalMatrix * aTangent);
    //    vec3 N = normalize(normalMatrix * aNormal);
    //    T = normalize(T - dot(T, N) * N);
    //    vec3 B = cross(N, T);
    //    mat3 TBN = transpose(mat3(T, B, N));
    //    vs_out.TangentLightPos = TBN * lightPos;
    //    vs_out.TangentViewPos  = TBN * viewPos;
    //    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
    //} else{
        Normal.xyz = transpose(inverse(mat3(finalInstanceMatrix))) * inNormal;
    //}
    worldPos.xyz = vec3(finalModel);
    gl_Position = ubo.projection * (ubo.view) * finalModel;
    materialIndex = mat;//22;
}
