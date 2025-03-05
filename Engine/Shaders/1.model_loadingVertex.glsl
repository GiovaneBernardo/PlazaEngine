#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 7) in mat4 aInstanceMatrix;
out vec2 TexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 worldPos;

} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool usingNormal;


void main()
{
/*
    vec3 objectPosition = vec3(0.0, 0.0, 0.0);
    mat4 translationMatrix = mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(objectPosition, 1.0)
    );
*/
    mat4 finalInstanceMatrix = aInstanceMatrix;
    vec4 model = finalInstanceMatrix * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(model);
    //vs_out.Normal = transpose(inverse(mat3(aInstanceMatrix))) * aNormal;
    vs_out.TexCoords = aTexCoords;

    if(usingNormal){
        mat3 normalMatrix = transpose(inverse(mat3(finalInstanceMatrix)));
        vec3 T = normalize(normalMatrix * aTangent);
        vec3 N = normalize(normalMatrix * aNormal);
        T = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T);
        mat3 TBN = transpose(mat3(T, B, N));
        vs_out.TangentLightPos = TBN * lightPos;
        vs_out.TangentViewPos  = TBN * viewPos;
        vs_out.TangentFragPos  = TBN * vs_out.FragPos;
    } else{
        vs_out.Normal = transpose(inverse(mat3(finalInstanceMatrix))) * aNormal;
    }
    vs_out.worldPos = vec3(model);
    gl_Position = projection * (view) * model;
}
