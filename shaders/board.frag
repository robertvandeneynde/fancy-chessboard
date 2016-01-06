#version 130

uniform int color;

uniform sampler2D normalMap;

uniform vec3 lights[4];
uniform vec3 lightColors[4];
uniform int nLights = 1; // one light is enough

in vec2 texCoord;
in vec3 position;
in vec3 normal;

out vec3 fragColor;

vec3 normalToColor(vec3 n) { return (n + 1) / 2; }
vec3 colorToNormal(vec3 c) { return c * 2 - 1; }

void main(void)
{
    vec3 normalMapVec = colorToNormal(texture2D(normalMap, (texCoord + vec2(0.5,0.5))).rgb);

    vec3 ambiant = vec3(0);

    vec3 N = normalize(normalMapVec);

    vec3 diffuse = vec3(0,0,0);

    for(int i = 0; i < nLights; i++) {
        vec3 L = normalize(lights[i] - position);
        diffuse += max(0, dot(L,N)) * lightColors[i];
    }

    vec3 myColor;
    if(color == 0)
        myColor = vec3(0.5); // vec3(0,0,1); // black
    else
        myColor = vec3(0.8); // vec3(1,0,0); // white

    fragColor = (ambiant + diffuse) * myColor;
    // fragColor = ((position/8)+1)/2; // normalMapVec;
    // fragColor = normalToColor(L);
}
