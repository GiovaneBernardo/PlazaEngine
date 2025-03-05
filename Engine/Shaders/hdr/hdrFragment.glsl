#version 330 core
in vec2 TexCoords;

uniform sampler2D hdrBuffer;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 brightTexture;
uniform bool hdr;
uniform float exposure;

void main()
{
    float expo = 0.75f;
    float gamma = 3.6f;
    // Fetch the HDR color from the texture
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;

    // Apply exposure adjustment
    vec3 adjustedColor = hdrColor * expo;

    // Apply gamma correction
    adjustedColor = hdrColor;//pow(adjustedColor, vec3(1.0 / gamma));

    float brightness = dot(adjustedColor, vec3(0.2126, 0.7152, 0.0722));

    if(adjustedColor.x > 1.0f || adjustedColor.y > 1.0f || adjustedColor.z > 1.0f)
    {
        brightTexture = vec4(adjustedColor, 1.0);
       // adjustedColor = vec3(1.0f);
    }
    else
        brightTexture = vec4(0.0, 0.0, 0.0, 0.0);

    // Output the final color
    FragColor = vec4(adjustedColor, 1.0);
}
