#version 460 core
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gOthers;
uniform sampler2D gDepth;
uniform sampler2D sceneColor;

uniform mat4 invView;
uniform mat4 projection;
uniform mat4 invProjection;
uniform mat4 view;

noperspective in vec2 TexCoords;

out vec4 outColor;

const float step = 0.05;
const float minRayStep = 01;
const float maxSteps = 180;
const int numBinarySearchSteps = 5;
const float reflectionSpecularFalloffExponent = 3.0;

uniform vec3 cameraPos;

float Metallic;

#define Scale vec3(.8, .8, .8)
#define K 19.19

vec3 BinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth);

vec4 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth);

vec3 fresnelSchlick(float cosTheta, vec3 F0);

vec3 hash(vec3 a);

void main()
{

    //vec2 MetallicEmmissive = vec2(texture2D(gOthers, TexCoords).z);
    vec2 MetallicEmmissive = vec2(1.0f);//texture2D(gOthers, TexCoords).rg;
    Metallic = MetallicEmmissive.r;

    if(texture(gPosition, TexCoords).y > 100)
        Metallic = 0;

    if(Metallic < 0.01 || texture(gDepth, TexCoords).x == 0.0)
    {
        outColor = texture(sceneColor, TexCoords);
        return;
    }

    vec3 viewNormal = vec3(normalize(texture2D(gNormal, TexCoords) * invView));//vec3(texture2D(gNormal, TexCoords) * (invView));
    vec3 viewPos = vec3(view * texture(gPosition, TexCoords));
    vec3 albedo = texture(sceneColor, TexCoords).rgb;

    float spec = texture(gOthers, TexCoords).r;
    //spec = 0.0f;
    vec3 F0 = vec3(0.04);
    F0      = mix(F0, albedo, Metallic);
    vec3 Fresnel = fresnelSchlick(max(dot(normalize(viewNormal), normalize(viewPos)), 0.0), F0);

    // Reflection vector
    vec3 reflected = normalize(reflect(normalize(viewPos), normalize(viewNormal)));

    vec3 hitPos = viewPos;
    float dDepth;

    vec3 wp = vec3(vec4(viewPos, 1.0) * invView);
    vec3 jitt = mix(vec3(0.0), vec3(hash(wp)), spec);
    vec4 coords = RayMarch((vec3(jitt) + reflected * max(minRayStep, -viewPos.z)), hitPos, dDepth);

    vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));

    float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

    float ReflectionMultiplier = pow(Metallic, reflectionSpecularFalloffExponent) *
                screenEdgefactor *
                -reflected.z;

    // Get color
    vec3 SSR = texture(sceneColor, coords.xy).rgb * clamp(ReflectionMultiplier, 0.0, 0.9) * Fresnel;
    SSR = texture2D(gDepth, coords.xy).x > 0.0 ? SSR : vec3(0.0f);

    outColor = vec4(albedo.rgb + SSR, 1.0f);
}

vec3 BinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth)
{
    float depth;

    vec4 projectedCoord;

    for(int i = 0; i < numBinarySearchSteps; i++)
    {

        projectedCoord = projection * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

        depth = (view * texture2D(gPosition, projectedCoord.xy)).z;

        dDepth = hitCoord.z - depth;

        dir *= 0.5;
        if(dDepth > 0.0)
            hitCoord += dir;
        else
            hitCoord -= dir;
    }

    projectedCoord = projection * vec4(hitCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

    return vec3(projectedCoord.xy, depth);
}

vec4 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth)
{

    dir *= step;


    float depth;
    int steps;
    vec4 projectedCoord;

    vec3 cameraPosScreen = (view * vec4(cameraPos, 1.0f)).xyz;

    for(int i = 0; i < maxSteps; i++)
    {
        hitCoord += dir;

        projectedCoord = projection * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

        depth = (view * texture2D(gPosition, projectedCoord.xy)).z;

        dDepth = hitCoord.z - depth;

        if(hitCoord.z > cameraPosScreen.z)
            return vec4(0.0f, 0.0f, 0.0f, 1.0f);

        if((dir.z - dDepth) < 1.2)
        {
            if(dDepth <= 0.0)
            {
                return vec4(BinarySearch(dir, hitCoord, dDepth), 1.0);
            }
        }

        steps++;
    }
    return (vec4(projectedCoord.xy, depth, 1.0f) * projection);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


vec3 hash(vec3 a)
{
    a = fract(a * Scale);
    a += dot(a, a.yxz + K);
    return fract((a.xxy + a.yxx)*a.zyx);
}
