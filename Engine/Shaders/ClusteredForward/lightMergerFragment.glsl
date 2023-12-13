#version 450 core
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gOthers;
uniform vec3 viewPos;
uniform float time;
uniform mat4 view;
uniform mat4 projection;

out vec4 FragColor;

in vec2 TexCoords;

struct LightStruct {
    vec3 color;
    float radius;
    vec3 position;
    float intensity;
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

layout (std430, binding = 0) buffer LightsBuffer {
    LightStruct lights[];
};

layout (std430, binding = 1) buffer ClusterBuffer {
    Cluster[] clusters;
};

layout (std430, binding = 3) buffer SizesBuffer {
    int[] sizes;
    //int[] sizes;
};

layout (std430, binding = 7) buffer FrustumsBuffer {
    Frustum frustums[];
};

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
    return round(value / multiple) * multiple;
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
    vec4 viewSpace = projection * (view * vec4(position, 1.0));
    vec3 ndcPosition = viewSpace.xyz / viewSpace.w;
    vec2 uvCoordinates = (ndcPosition.xy + 1.0) / 2.0;
    return uvCoordinates;
}

int GetIndex(vec2 screenPos, vec2 clusterCount)
{
    vec2 currentClusterPosition = vec2(ceil(roundToMultiple(screenPos.x * screenSize.x, clusterSize.x) / clusterSize.x), ceil(roundToMultiple(screenPos.y * screenSize.y, clusterSize.y) / clusterSize.y));
    return int(((currentClusterPosition.y * (clusterCount.x)) + (currentClusterPosition.x)));
}

#define MAX_POINT_LIGHT_PER_TILE 2048

void main()
{  
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gDiffuse, TexCoords).xyz;
    float Specular = texture(gOthers, TexCoords).x;
    float metalness = texture(gOthers, TexCoords).y;
    float roughness = texture(gOthers, TexCoords).z;
    vec3 spe = texture(gOthers, TexCoords).xyz;
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
    float radius = 5.01f;
    vec2 clusterCount = ceil(screenSize / clusterSize.xy);
    vec4 viewSpace = view * vec4(FragPos, 1.0);
    vec2 viewSpaceCoords = viewSpace.xy / viewSpace.w;
    vec2 currentClusterPosition = vec2(ceil(roundToMultiple(TexCoords.x * screenSize.x, clusterSize.x) / clusterSize.x), ceil(roundToMultiple(TexCoords.y * screenSize.y, clusterSize.y) / clusterSize.y));
    int clusterIndex = int(((currentClusterPosition.y * (clusterCount.x)) + (currentClusterPosition.x)));
   // clusterIndex = GetGridIndex(viewSpaceCoords);
   clusterIndex = GetIndex(TexCoords, clusterCount);

    vec2 clusterIndexXY = indexToRowCol(clusterIndex, clusterCount);

    Cluster currentCluster = clusters[clusterIndex];

    /*
                vec4 viewSpace = view * vec4(lights[i].position.xyz, 1.0);
            vec2 viewSpaceCoords = viewSpace.xy / viewSpace.w;
            int clusterIndex = int((viewSpaceCoords.x * screenSize.x / clusterSize.x) * (viewSpaceCoords.y * screenSize.y / clusterSize.y));
    */
    for (int i = 0; i < MAX_POINT_LIGHT_PER_TILE && currentCluster.lightsIndex[i] != -1; ++i)
    {
        LightStruct light = lights[currentCluster.lightsIndex[i]];
        //light.color = vec3(1.0f, 0.0f, 0.0f);
        vec3 lightPosition = light.position.xyz;

        // calculate distance between light source and current fragment
        float distance = length(lightPosition - FragPos);
        //if(distance < radius * radius)
        //if(distance < light.radius)
        if(distance < light.radius)
        {
            vec3 lightColor = light.color;
            // diffuse
            vec3 lightDir = normalize(lightPosition - FragPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * (lightColor * light.intensity);
            // specular
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
            vec3 specular = (lightColor * light.intensity) * spec * Specular;
            // attenuation
            float constant = 1.0f;
            float quadratic = 1.8f;
            float linear = 0.7f;
            float attenuation = 1.0 / (1.0 + linear * distance + quadratic * distance * distance);
            attenuation *= (light.radius / 1);
            diffuse *= attenuation;
            specular *= attenuation;
            lighting += (diffuse + specular); //* attenuation;
        }
     }    
    //lighting += 1.0f;

    #if 1
    vec2 co = TexCoords * screenSize;// (clusterSize.xy );//* clusterSize.xy);
    vec2 pos = co.xy;
    const float w = screenSize.x;
    const uint gridIndex = uint(GetGridIndex(pos));
    const Frustum f = frustums[gridIndex];
    const float halfTile = 32 / 2; // (screenSize.x);//clusterSize.x / 2 / screenSize.x);
    const float halfTileY = halfTile; // (screenSize.y);//clusterSize.x / 2 / screenSize.x);
    vec3 color = abs(f.planes[1].Normal);
    if(GetGridIndex(vec2(pos.x + halfTile, pos.y)) == gridIndex && GetGridIndex(vec2(pos.x, pos.y + halfTileY)) == gridIndex)
    {
        color = abs(f.planes[0].Normal);
    }
    else if(GetGridIndex(vec2(pos.x + halfTile, pos.y)) != gridIndex && GetGridIndex(vec2(pos.x, pos.y + halfTileY)) == gridIndex)
    {
        color = abs(f.planes[2].Normal);
    }
    else if(GetGridIndex(vec2(pos.x + halfTile, pos.y)) == gridIndex && GetGridIndex(vec2(pos.x, pos.y + halfTileY)) != gridIndex)
    {
        color = abs(f.planes[3].Normal);//abs(f.planes[3].Normal);
    }
    //color = mix(Diffuse, color, 0.1f);
    #endif

    //color.x = GetGridIndex(vec2(pos.x + halfTile, pos.y));
    //color.y = GetGridIndex(vec2(pos.x, pos.y));
    clusterIndexXY = pos / 32;
    float c = (clusterIndexXY.x + clusterCount.x * clusterIndexXY.y) * 0.00001f;
    if(int(clusterIndexXY.x) % 2 == 0) c += 0.1f;
    if(int(clusterIndexXY.y) % 2 == 0) c += 0.1f;

    vec3 heatmap = HeatMap(clusterIndex, currentCluster.lightsCount).xyz;
    heatmap = heatmap == vec3(0.0f, 1.0f, 0.0f) ? vec3(0.0f, 0.0f, 0.5f) : heatmap;
    //FragColor = vec4(c, c, c, 1.0f);
    //FragColor = vec4(mix(Diffuse * lighting, heatmap, 0.8f), 1.0f);
    //FragColor = vec4(heatmap, 1.0f);
    //FragColor = vec4(color, 1.0f);
    FragColor = vec4(Diffuse + lighting, 1.0f);
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