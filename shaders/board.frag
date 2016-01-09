#version 130

uniform int color;
uniform vec3 camera;

uniform sampler2D normalMap;
uniform samplerCube cubemap;
uniform float shininess = 32;

uniform vec3 lights[4];
uniform vec3 lightColors[4];
uniform int nLights = 1;
uniform float reflectFactor = 0.2;
uniform float refractFactor = 0.1;
uniform float refractIndice = 0.2;

in vec2 texCoord;
in vec3 position;
in vec3 normal;

out vec4 fragColor;

vec3 normalToColor(vec3 n) { return (n + 1) / 2; }
vec3 colorToNormal(vec3 c) { return c * 2 - 1; }

void main(void)
{
    vec3 normalMapVec = colorToNormal(texture2D(normalMap, texCoord + vec2(0.5,0.5)).rgb);

    vec3 ambiant = vec3(1)/5.0;

    vec3 N = normalize(normalMapVec);
    vec3 V = normalize(camera - position);

    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);

    for(int i = 0; i < nLights; i++) {
        vec3 L = normalize(lights[i] - position);
        vec3 R = normalize(2 * dot(L,N) * N - L);

        diffuse += max(0, dot(L,N)) * lightColors[i];
        specular += pow(max(0, dot(R,V)), shininess) * lightColors[i];
    }

    vec3 myColor;
    if(color == 0)
        myColor = vec3(0.29,0.15,0); // vec3(0,0,1); // black
    else
        myColor = vec3(0.8); // vec3(1,0,0); // white

    vec4 refl = reflectFactor * texture(cubemap, reflect(-V,N));
    vec4 refr = refractFactor * texture(cubemap, refract(-V,N,refractIndice));

    fragColor = vec4((ambiant + diffuse + specular) * myColor, 1) +  refl + refr;
    // fragColor = ((position/8)+1)/2; // normalMapVec;
    // fragColor = normalToColor(L);
}
