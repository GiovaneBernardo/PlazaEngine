#version 450

//layout (binding = 0) uniform sampler2D gPosition;
layout (binding = 0) uniform sampler2D gNormal;
layout (binding = 1) uniform sampler2D gDiffuse;
layout (binding = 2) uniform sampler2D gOthers;
layout (binding = 3) uniform sampler2D gSceneDepth;

layout(push_constant) uniform PushConstants {
    vec3 viewPos;
    float time;
    mat4 view;
    mat4 inverseView;
    mat4 projection;
    int lightCount;
    vec4 ambientLightColor;
} pushConstants;

layout(location = 0) out vec4 FragColor;
layout(location = 1) in vec2 TexCoords;

struct LightStruct {
    vec3 color;
    float radius;
    vec3 position;
    float intensity;
    float cutoff;
};

struct Cluster{
    int[256] lightsIndex;
    int lightsCount;
    vec3 minBounds;
    vec3 maxBounds;
};

struct Plane
{
    vec3 Normal;
    float Distance;
};

struct Frustum
{
    Plane planes[4];
};

layout (std430, binding = 4) buffer LightsBuffer {
    LightStruct lights[];
};

layout (std430, binding = 5) buffer ClusterBuffer {
    Cluster[] clusters;
};

//layout (std430, binding = 6) buffer FrustumsBuffer {
//    Frustum frustums[];
//};

float attenuate(vec3 lightDirection, float radius) {
	float cutoff = .99;
	float attenuation = dot(lightDirection, lightDirection) / (100.0 * radius);
	attenuation = 1.0 / (attenuation * 15.0 + 1.0);
	attenuation = (attenuation - cutoff) / (cutoff);

	return clamp(attenuation, 0.0, 1.0);
}

vec2 screenSize = vec2(1820, 720);

vec3 clusterSize = vec3(32, 32, 32);

int roundUp(float numToRound, float multiple)
{
numToRound = round(numToRound);
    if (multiple == 0)
        return int(numToRound);

    int remainder = int(numToRound) % int(multiple);
    if (remainder == 0)
        return int(round(numToRound));

    return int(round(numToRound + multiple - remainder));
}

float roundToMultiple(float value, float multiple) {
    return floor(value / multiple) * multiple;
}

vec2 indexToRowCol(int index, vec2 clusterCount) {
    float col = mod(float(index), clusterCount.x);
    float row = floor(float(index) / clusterCount.x);
    return vec2(row, col);
}

int GetGridIndex(vec2 posXY)
{
    const vec2 pos = vec2(uint(posXY.x), uint(posXY.y));
    const uint tileX = uint(ceil(screenSize.x / clusterSize.x));
    return int((pos.x / clusterSize.x) + (tileX * (pos.y / clusterSize.x)));
}

vec4 HeatMap(int clusterIndex, int numLights)
{
    vec3 color;
    if(numLights <= 0)
    {
        color = vec3(0.5f, 0.5f, 0.5f);    
    }
    else if(numLights < 10)
    {
        color = vec3(0.2f, 1.0f, 0.0f);    
    }
    else if(numLights < 50)
    {
        color = vec3(0.4f, 0.8f, 0.0f);    
    }
    else if(numLights < 100)
    {
        color = vec3(0.8f, 0.4f, 0.0f);    
    }
    else if(numLights >= 100)
    {
        color = vec3(1.0f, 0.0f, 0.0f);    
    }
    return vec4(color, 1.0f);
}

vec2 worldToScreen(vec3 position){
    vec4 viewSpace = pushConstants.projection * (pushConstants.view * vec4(position, 1.0));
    vec3 ndcPosition = viewSpace.xyz / viewSpace.w;
    vec2 uvCoordinates = (ndcPosition.xy + 1.0) / 2.0;
    return uvCoordinates;
}

int GetIndex(vec2 screenPos, vec2 clusterCount)
{
    vec2 currentClusterPosition = vec2(ceil(roundToMultiple(screenPos.x * screenSize.x, clusterSize.x) / clusterSize.x), ceil(roundToMultiple(screenPos.y * screenSize.y, clusterSize.y) / clusterSize.y));
    return int(((currentClusterPosition.y * (clusterCount.x)) + (currentClusterPosition.x)));
}

const float PI = 3.14159265359;

float D_GGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2)/(PI * denom*denom); 
}

float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

vec3 F_Schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}


vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness, vec3 materialColor)
{
    vec3 H = normalize (V + L);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);

    vec3 color = vec3(0.0);

    if (dotNL > 0.0) {
        float D = D_GGX(dotNH, roughness); 
        float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
        vec3 F = F_Schlick(dotNV, F0);		
        vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);		
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);			
        color += (kD * materialColor / PI + spec) * dotNL;
    }

    return color;
}

#define MAX_POINT_LIGHT_PER_TILE 2048
//#define SHOW_HEATMAP

void main()
{  
    // retrieve data from gbuffer
    vec3 Diffuse = texture(gDiffuse, TexCoords).xyz;
    // then calculate lighting as usual
    vec3 lighting  = vec3(0.0f);//Diffuse * 0.1; // hard-coded ambient component


    if(pushConstants.lightCount > 0) {
        // Reconstruct world-space position from depth
        vec2 screenPos = (gl_FragCoord.xy / screenSize) * 2.0 - 1.0;
        float depth = texture(gSceneDepth, TexCoords).r;
        vec4 clipSpacePos = vec4(screenPos.xy, depth, 1.0);
        vec4 viewSpacePos = pushConstants.inverseView * clipSpacePos;
        viewSpacePos /= viewSpacePos.w;
        vec3 FragPos = (pushConstants.inverseView * viewSpacePos).xyz;

        // Reconstruct normal from 2-component packed normal
        vec2 encodedNormal = texture(gNormal, TexCoords).xy;
        vec3 Normal;
        Normal.xy = encodedNormal;
        Normal.z = sqrt(1.0 - dot(Normal.xy, Normal.xy));
        Normal = normalize(Normal);

        //vec3 FragPos = texture(gPosition, TexCoords).rgb;
        //vec3 Normal = texture(gNormal, TexCoords).rgb;
        float Specular = texture(gOthers, TexCoords).x;
        float metalness = texture(gOthers, TexCoords).y;
        float roughness = texture(gOthers, TexCoords).z;
        vec3 spe = texture(gOthers, TexCoords).xyz;

        vec2 clusterCount = ceil(screenSize / clusterSize.xy);
        int clusterIndex = GetIndex(TexCoords, clusterCount);
        Cluster currentCluster = clusters[clusterIndex];

        vec3 viewDir  = normalize(pushConstants.viewPos - FragPos);

        vec3 F0 = vec3(0.04); 
        F0 = mix(F0, Diffuse, metalness);

        vec3 V = normalize(pushConstants.viewPos.xyz - (FragPos.xyz));

        for (int i = 0; i < MAX_POINT_LIGHT_PER_TILE && clusters[clusterIndex].lightsIndex[i] != -1; ++i) {

            LightStruct light = lights[clusters[clusterIndex].lightsIndex[i]];
            //light.color = vec3(1.0f, 0.0f, 0.0f);
            vec3 lightPosition = light.position.xyz;

            // calculate distance between light source and current fragment
            float distance = length(lightPosition - FragPos);

            if(distance < light.radius)
            {

                vec3 L = normalize(-lightPosition.xyz - FragPos); // Directional light direction


                //vec3 L = lightPosition.xyz - FragPos;
		        float dist = length(L);
                
		        L = normalize(L);
		        float atten = light.radius / (pow(dist, light.cutoff) + 1.0);
                
		        vec3 N = normalize(Normal);
		        float NdotL = max(0.0, dot(N, L));
                vec3 lightColor = light.color * light.intensity;
		        vec3 diff = min(lightColor * Diffuse * NdotL * atten, lightColor);
                //float atten = light.radius / (pow(dist, light.cutoff) + 1.0);
                vec3 Lo = specularContribution(L, V, Normal, F0, metalness, roughness, diff);
                lighting += diff + Lo * atten;
            }
        }    
     }
    //lighting += 1.0f;
    vec3 color;

//#ifdef SHOW_HEATMAP
//    vec2 co = TexCoords * screenSize;// (clusterSize.xy );//* clusterSize.xy);
//    vec2 pos = co.xy;
//    const float w = screenSize.x;
//    const uint gridIndex = uint(GetGridIndex(pos));
//    const Frustum f = frustums[gridIndex];
//    const float halfTile = 32 / 2; // (screenSize.x);//clusterSize.x / 2 / screenSize.x);
//    const float halfTileY = halfTile; // (screenSize.y);//clusterSize.x / 2 / screenSize.x);
//    color = abs(f.planes[1].Normal);
//    if(GetGridIndex(vec2(pos.x + halfTile, pos.y)) == gridIndex && GetGridIndex(vec2(pos.x, pos.y + halfTileY)) == gridIndex)
//    {
//        color = abs(f.planes[0].Normal);
//    }
//    else if(GetGridIndex(vec2(pos.x + halfTile, pos.y)) != gridIndex && GetGridIndex(vec2(pos.x, pos.y + halfTileY)) == gridIndex)
//    {
//        color = abs(f.planes[2].Normal);
//    }
//    else if(GetGridIndex(vec2(pos.x + halfTile, pos.y)) == gridIndex && GetGridIndex(vec2(pos.x, pos.y + halfTileY)) != gridIndex)
//    {
//        color = abs(f.planes[3].Normal);//abs(f.planes[3].Normal);
//    }
//
//    clusterIndexXY = pos / 32;
//    float c = (clusterIndexXY.x + clusterCount.x * clusterIndexXY.y) * 0.00001f;
//    if(int(clusterIndexXY.x) % 2 == 0) c += 0.1f;
//    if(int(clusterIndexXY.y) % 2 == 0) c += 0.1f;
//
//    vec3 heatmap = HeatMap(clusterIndex, currentCluster.lightsCount).xyz;
//    heatmap = heatmap == vec3(0.0f, 1.0f, 0.0f) ? vec3(0.0f, 0.0f, 0.5f) : heatmap;
//
//
//    color = mix(Diffuse + lighting, heatmap, 0.8f);
//
//#else
    color = Diffuse + lighting;
    //const float maxFogDistance = 2000.0f;
    //float fogDensity = 1.0f / maxFogDistance;
    //float fogFactor = exp(-fogDensity * length(pushConstants.viewPos - FragPos));
    //color = mix(vec3(0.5f, 0.8f, 1.0f) * 1.0f, color, fogFactor);
//#endif





    FragColor = vec4(color, 1.0f);
    //FragColor = vec4(currentCluster.lightsCount / MAX_POINT_LIGHT_PER_TILE, 0.0f, 0.0f, 1.0f);
    //FragColor = vec4(int(currentClusterPosition.x) % 2 == 0 && int(currentClusterPosition.y) % 2 == 0 ? 1.0f : 0.0f, 0.0f, 0.0f, 1.0f);
    //FragColor = vec4(currentClusterPosition.x, currentClusterPosition.y, clusterIndex, 1.0f);
    //FragColor = vec4(clusterColorDebugg, clusterIndex);
    //FragColor = vec4(clusterIndex, clusterIndex, clusterIndex, 1.0f);


    //FragColor = vec4(0.2f) + vec4(mod(viewSpaceCoords.x, clusterSize.x), mod(viewSpaceCoords.y, clusterSize.y), 0.0f, 1.0);
    //FragColor = vec4(attenuation, 1.0f);//vec4(Diffuse, 1.0);
    //FragColor = vec4(pow(Diffuse, vec3(2.2f)) * vec3(1 + texture(lightTexture, TexCoords)), 1.0);
}


//     FragColor = vec4(Diffuse * lighting, 1.0f);