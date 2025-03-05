#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 7) in mat4 aInstanceMatrix;
out vec2 TexCoords;

out VS_OUT {
    vec3 normal;
} vs_out;

uniform mat4 view;
uniform mat4 model;

void main()
{
    mat4 finalInstanceMatrix = aInstanceMatrix;
    gl_Position = view * finalInstanceMatrix * vec4(aPos, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(view * finalInstanceMatrix)));
    vs_out.normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
}
