#version 330 core

// Input textures
uniform sampler2D buffer1; // Blurred edges texture
uniform sampler2D buffer2; // Scene texture

// Texture coordinates
in vec2 TexCoord;

out vec4 FragColor;


uniform sampler2D Depth0;
uniform sampler2D Depth1;

/*
void main()
{
    ivec2 texcoord = ivec2(floor(gl_FragCoord.xy));
    float depth0 = texelFetch(Depth0, texcoord, 0).r;
    float depth1 = texelFetch(Depth1, texcoord, 0).r;

    // possibly reversed depending on your depth buffer ordering strategy
    if (depth0 < depth1) {
        FragColor = texelFetch(buffer1, texcoord, 0);
    } else {
        FragColor = texelFetch(buffer2, texcoord, 0);
    }
}
*/
/*
void main()
{
    vec4 silhouetteColor = texture(buffer1, TexCoord);
    vec4 sceneColor = texture(buffer2, TexCoord);

    // Perform blending operation based on silhouette presence
    vec4 blendedColor = silhouetteColor != vec4(0.0) ? silhouetteColor : sceneColor;

    FragColor = blendedColor;
}
*/

void main()
{
    // Sample the blurred edges and scene textures
    vec4 edgesColor = texture(buffer1, TexCoord);
    vec4 sceneColor = texture(buffer2, TexCoord);

    // Combine the colors using a desired blending operation
    //vec4 finalColor = edgesColor * 0.5 + sceneColor * 0.5; // Example: linear interpolation
    vec4 finalColor;
    if(texture(buffer1, TexCoord) != vec4 (0.0,0.0,0.0,0.0)){
        finalColor = texture(buffer1, TexCoord);
    } else {
        finalColor = texture(buffer2, TexCoord);
    }


    if(texture(buffer2, TexCoord) == vec4(0.6, 0.3, 0.3, 1.0)){
        FragColor = vec4(pow(vec3(1.0, 0.4f, 0.2f), vec3(2.2f)), 1.0f);
    } else {
        FragColor = texture(buffer1, TexCoord);
    }
      //  finalColor = mix(texture(buffer1, TexCoord), texture(buffer2, TexCoord), 1.0);
    //FragColor = finalColor;

}
