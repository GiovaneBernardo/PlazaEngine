#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
    uniform sampler2D depthStencilTexture;
    uniform sampler2D depthStencilTexture2;

const float offset = 1.0 / 900.0;

void main()
{
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right
    );

float kernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16
);

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

    FragColor = vec4(col, 1.0);

    vec2 screenCoords = gl_FragCoord.xy;
    //float currentDepth = gl_FragCoord.z;

    float depthValue = texture(depthStencilTexture, screenCoords).r;
    float currentDepth = texture(depthStencilTexture2, screenCoords).r;
            if (texture(screenTexture, TexCoords) == vec4(0.0, 0.0, 0.0, 1.0))
        {
            // Fragment is behind or at the same depth as the one in the depth buffer
            // Output desired color here
            //FragColor = vec4(0.3, 0.5, 0.7, 1.0);  // Yellow color
                FragColor = vec4(col, 1.0);
                if(FragColor != vec4(0.0, 0.0, 0.0, 1.0)){
                    FragColor = vec4(0.6, 0.3, 0.3, 1.0);
                }
        }
        else
        {
            // Fragment is in front of the one in the depth buffer
            //discard;
            FragColor = vec4(0.0, 0.0, 0.0, 0.0);
        }
    //FragColor = texture(screenTexture, TexCoords);
}
