#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    vec3 text1 = texture(texture1, TexCoords).rgb;
    vec3 text2 = texture(texture2, TexCoords).rgb;
     FragColor = vec4(text1 + text2, 1.0);
}
