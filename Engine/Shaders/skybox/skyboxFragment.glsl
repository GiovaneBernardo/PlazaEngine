#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    //FragColor = vec4(0.5, 0.6, 0.6, 1.0);
    FragColor = vec4(pow(texture(skybox, TexCoords).rgb, vec3(2.2f)) * 2.2f , 1.0f);
}
