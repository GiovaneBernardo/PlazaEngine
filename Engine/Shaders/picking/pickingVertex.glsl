#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;


out float objectID;
uniform uint uuidArray[2];
flat out uint array[2];

uniform float pixelObjectID;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
    objectID = pixelObjectID;
    array[0] = uuidArray[0];
    array[1] = uuidArray[1];
}
