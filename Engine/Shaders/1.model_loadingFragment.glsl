#version 410 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gDiffuse;
layout (location = 3) out vec4 gOthers;
layout (location = 4) out vec4 gDepth;

out vec4 FragColor;

uniform bool usingNormal;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_normal;
uniform sampler2D texture_height;
uniform sampler2D texture_metalness;
uniform sampler2D texture_roughness;
uniform float shininess;
uniform float intensity;

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
uniform vec3 lightDirection;


layout (std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[32];
};

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 worldPos;
} fs_in;

uniform vec3 lightPos;
uniform vec3 viewPos;
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
    vec3 normal;
    if(usingNormal){
        normal = texture(texture_normal, fs_in.TexCoords).rgb;
        normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
    } else {
        normal = normalize(fs_in.Normal);
    }
    float bias = max(0.0005 * (1.0 - dot(normal, lightDirection)), 0.00005);
    const float biasModifier = 0.5f;
    if (layer == cascadeCount)
    {
        bias *= 1 / (farPlane * biasModifier);
    }
    else
    {
        bias *= 1 / ((cascadePlaneDistances[layer]) * biasModifier);
    }
    float distanceToCamera = distance(viewPos, projCoords.xyz);

    bias = 0.0001;
    float floatVal = 3 - (texture(shadowsDepthMap, vec3(projCoords.xyz)).r * 2.3);
    int pcfCount = 5;// + int(floatVal);
    float mapSize = 4096.0 * 4;
    float texelSize = (1.0 / mapSize * 2) * floatVal;
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

const float PI = 3.14159265359;
 float metallic = 0;
 float roughness = 0.10f;
uniform float ao = 0;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

vec3 schlickFresnel(float vDotH, vec3 color)
{
    vec3 F0 = vec3(0.04);
    F0 = color;

    vec3 ret = F0 + (1 - F0) * pow(clamp(1.0 - vDotH, 0.0, 1.0), 5);

    return ret;
}


float geomSmith(float dp, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float denom = dp * (1 - k) + k;
    return dp / denom;
}


float ggxDistribution(float nDotH, float roughness)
{
    float alpha2 = roughness * roughness * roughness * roughness;
    float d = nDotH * nDotH * (alpha2 - 1) + 1;
    float ggxdistrib = alpha2 / (PI * d * d);
    return ggxdistrib;
}
float far = 15000.0f;
float near = 0.01f;
float LinearizeDepth(float depth) {
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

uniform mat4 model;

#define COLOR_MAX_MIN 65000

void main()
{
    vec3 color = texture(texture_diffuse, fs_in.TexCoords).rgb;
    if(texture_diffuse_rgba != vec4(300, 300, 300, 300)){
         color = clamp(texture_diffuse_rgba.rgb, -COLOR_MAX_MIN, COLOR_MAX_MIN);
    }
    else{
        color = texture(texture_diffuse, fs_in.TexCoords).rgb;
    }
    color = color * intensity;

    vec3 lightColor = vec3(1.0f, 0.85f, 0.85f) * 255;
    // ambient
    vec3 ambient = 1.32 * (lightColor / 1);
    // diffuse

    metallic = pow(texture(texture_metalness, fs_in.TexCoords) / 1, vec4(1/ 2.2)).r * 1;
    roughness = pow(texture(texture_roughness, fs_in.TexCoords) / 1, vec4(1/ 2.2)).r * 1;
    //metallic = texture(texture_metalness, fs_in.TexCoords).r / 255;//pow(texture(texture_metalness, fs_in.TexCoords), vec4(2.2)).r;
    //roughness = texture(texture_metalness, fs_in.TexCoords).r / 255;//pow(texture(texture_roughness, fs_in.TexCoords) / 1, vec4(2.2)).r;
    //metallic *= 2;
    //metallic = 0.5 * 1;
    //roughness = 0.3f * 1;
    vec3 normal;
    vec3 lightDir;
    float diff;
    vec3 diffuse;
    vec3 viewDir;
    if(usingNormal){
        normal = texture(texture_normal, fs_in.TexCoords).rgb;
        normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
        //lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
        diff = max(dot(lightDirection, normal), 0.0);
        viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    } else {
        normal = normalize(fs_in.Normal);
        //lightDir = normalize(lightPos - fs_in.FragPos);
        diff = max(dot(lightDirection, normal), 0.0);
        viewDir = normalize(viewPos - fs_in.FragPos);
    }

    vec3 l = normalize(lightDirection);
    vec3 n = normal;
    vec3 v = normalize(viewPos - fs_in.worldPos);
    vec3 h = normalize(v + l);

    float nDotH = max(dot(n, h), 0.0);
    float vDotH = max(dot(v, h), 0.0);
    float nDotL = max(dot(n, l), 0.0);
    float nDotV = max(dot(n, v), 0.0);

    vec3  F0 = mix (vec3 (0.04), color, metallic);
    vec3 F = fresnelSchlick(vDotH, F0);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;

    vec3 SpecBRDF_nom  = ggxDistribution(nDotH, roughness) *
                         F *
                         geomSmith(nDotL, roughness) *
                         geomSmith(nDotV, roughness);

    float SpecBRDF_denom = 4.0 * nDotV * nDotL + 0.0001;

    vec3 SpecBRDF = SpecBRDF_nom / SpecBRDF_denom;

    vec3 fLambert = vec3(0.0);
    fLambert = color * 1;

    vec3 DiffuseBRDF = kD * fLambert / PI;

    float shadow = ShadowCalculation(fs_in.FragPos);
    vec3 amb = vec3(0.16f);
    vec3 shad = (amb + (1 - shadow) * 2);
    //shad += ambient;
    shad /= 1;
    float specularIntensity = 13.0f;
    gOthers = vec4(SpecBRDF * specularIntensity, 1.0f);
    gOthers.z = metallic;
    SpecBRDF = shadow == 0 ? SpecBRDF : vec3(0);
    vec3 FinalColor = (shad + (DiffuseBRDF + SpecBRDF * specularIntensity)) * color * (nDotL + amb / 2);//((DiffuseBRDF)) * (shad / 255) * lightColor * nDotL * (vec3(0.3 / 255) * lightColor);

    //FinalColor += vec3(0.13f / 255);
    //FinalColor *= vec3(1);
    //FinalColor -= shadow;

//    FinalColor = FinalColor / (FinalColor + vec3(1.0));
    //FinalColor *= ambient;
    //FinalColor *= (1 - shadow) * 1;
    //FinalColor += vec3(0.01f);

    /* Geometry */
    gPosition = vec4(fs_in.FragPos, 1.0f);
    gDiffuse = vec4(FinalColor, 1.0f);

    if(usingNormal)
    {
        vec3 T = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
        vec3 B = cross(normal, T);
        mat3 TBN = mat3(T, B, normal);
        vec3 normalB = normalize(TBN * normal);
        // Transform normal to world space
        //normalB = normalize((view * model * vec4(normal, 0.0)).xyz);
        vec3 normalWorld = normalize((view * model * vec4(normalB, 1.0f)).xyz);
        gNormal = vec4(normalWorld, 1.0f);
    }
    else
        gNormal = vec4(normalize(normal), 1.0f);


    vec3 reflectDir = reflect(-lightDir, normal);
    //gOthers.r = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);

    float dep = gl_FragCoord.z / gl_FragCoord.w / far;//LinearizeDepth(gl_FragCoord.z) / far;//
    gDepth = vec4(dep, LinearizeDepth(gl_FragCoord.z / gl_FragCoord.w) / far, gl_FragCoord.z, 1.0f);//gl_FragCoord.z / gl_FragCoord.w;
    //gDepth.x = gDepth.y;
     //gPosition = vec4(1.0f, 0.2f, 0.2f, 1.0f);
    // Gamma correction
    vec4 FinalLight = vec4(FinalColor, 1.0);

    // calculate shadow
    FragColor = vec4(FinalLight.xyz, 1.0f);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
