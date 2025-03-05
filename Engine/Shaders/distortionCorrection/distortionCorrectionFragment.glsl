#version 330 core
in vec2 TexCoords;
uniform sampler2D sceneTexture;
out vec4 FragColor;

float distortionAmount = 0.0; // Adjust this to control the amount of distortion

void main()
{
    // Calculate the normalized coordinates [-1, 1] based on TexCoords
    vec2 normalizedCoords = 2.0 * TexCoords - 1.0;

    // Apply barrel distortion correction
    float r = length(normalizedCoords);
    float distortionFactor = 1.0 + distortionAmount * r * r;
    vec2 correctedCoords = normalizedCoords * distortionFactor * 0.5 + 0.5;

    // Sample the scene texture using the corrected coordinates
    vec4 frag = texture(sceneTexture, correctedCoords);
    FragColor = vec4(pow(frag.rgb, vec3(1.0f / 2.2f)), 1.0f);
}
