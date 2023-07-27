#version 330 core

out vec4 FragColor;


uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_normal;
uniform sampler2D texture_height;
uniform float shininess;

uniform vec4 texture_diffuse_rgba = vec4(300, 300, 300, 300);
uniform bool texture_diffuse_rgba_bool;

uniform vec4 texture_specular_rgba = vec4(300, 300, 300, 300);
uniform bool texture_specular_rgba_bool;

uniform sampler2DArray shadowsDepthMap;
uniform float farPlane;
uniform float cascadePlaneDistances[32];
uniform int cascadeCount;   // number of frusta - 1
uniform mat4 view;
uniform vec3 lightDir;

layout (std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[32];
};

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float objectID;
out vec3 pixelObjectID;


float ShadowCalculation(vec3 fragPosWorldSpace)
{
    // select cascade layer
    vec4 fragPosViewSpace = view * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);
    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (depthValue < cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = cascadeCount;
    }

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    const float biasModifier = 0.5f;
    if (layer == cascadeCount)
    {
        bias *= 1 / (farPlane * biasModifier);
    }
    else
    {
        bias *= 1 / (cascadePlaneDistances[layer] * biasModifier);
    }

    int pcfCount = 5;
    float mapSize = 4096.0 * 4;
    float texelSize = 1.0 / mapSize;
    float total = 0.0;
    float totalTexels = (pcfCount * 2.0 + 1.0) * (pcfCount * 2.0 + 1.0);
    // PCF
    float shadow = 0.0;
    //vec2 texelSize = 1.0 / vec2(textureSize(shadowsDepthMap, 0));
    for(int x = -pcfCount; x <= pcfCount; ++x)
    {
        for(int y = -pcfCount; y <= pcfCount; ++y)
        {
            float pcfDepth = texture(shadowsDepthMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= totalTexels;
        
    return shadow;
}

void main()
{       
    vec3 color = texture(texture_diffuse, fs_in.TexCoords).rgb;
    if(texture_diffuse_rgba != vec4(300, 300, 300, 300)){
         color = texture_diffuse_rgba.rgb;
    }
    else{
        color = texture(texture_diffuse, fs_in.TexCoords).rgb;
    }

    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(1.0);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 texSpec;
    if(texture_specular_rgba != vec4(300, 300, 300, 300)){
         texSpec = texture_specular_rgba.rgb;
    }
    else{
        texSpec = texture(texture_specular, fs_in.TexCoords).rgb;
    }
    vec3 specular = (spec * texSpec) * lightColor;       

    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPos);                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);

        pixelObjectID = vec3(3, 3, 3);
/*
    vec3 color = texture(texture_diffuse, fs_in.TexCoords).rgb;

    
    vec3 texDiffuseCol = texture2D(texture_diffuse, fs_in.TexCoords).rgb;

    if(texture_diffuse_rgba_bool){
         color = texture_diffuse_rgba.rgb;
    }
    else{
        color = texture(texture_diffuse, fs_in.TexCoords).rgb;
    }

    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(1.0);
    // ambient
    vec3 ambient = 0.15 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = lightColor * (diff * color);
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    vec3 texSpec;
    if(texture_specular_rgba_bool){
         texSpec = texture_specular_rgba.rgb;
    }
    else{
        texSpec = texture(texture_specular, fs_in.TexCoords).rgb;
    }
    vec3 specular = (spec * texSpec) * lightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
    */
}
