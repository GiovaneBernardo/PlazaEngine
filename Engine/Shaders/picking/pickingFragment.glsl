#version 330 core

out uvec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

flat in uint array[2];
uniform float objectID;
out uvec3 pixelObjectID;

void main()
{
    uvec3 color = uvec3(uint(array[0]), uint(array[1]), 0u);
    pixelObjectID = color;
    FragColor = uvec4(color.x, color.y, 1.0f, 1.0f);
}
