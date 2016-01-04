#version 130

uniform vec3 camera;
uniform vec3 light;

uniform sampler2D diag;
uniform sampler2D bump;

in vec3 normal;
in vec3 position;
in vec3 outNormal;
in vec3 outColor;
in vec2 outCoord;

uniform mat3 normMatrix;

out vec3 color;

vec3 normalToColor(vec3 n) {
    return (n + 1) / 2;
}

vec3 colorToNormal(vec3 c) {
    return c * 2 - 1;
}

void main()
{
    vec3 constColor = vec3(0,0,0.5);
    vec3 bumpColor = texture2D(bump, outCoord).rgb;
    // bumpColor = colorToNormal(bumpColor); // from [0,1] to [-1,1]
    vec3 realColor = texture2D(diag, outCoord).rgb;
    // realColor = vec3(1,0,0); // outColor;
    vec3 L = normalize(light - position);
    vec3 N = normalize(normMatrix * normalize(bumpColor)); // take normal from bump map
    // N = normalize(normMatrix * normalize(normal)); // simply take normal

    vec3 V = normalize(camera - position);
    vec3 R = normalize(2 * dot(L,N) * N - L);

    float alpha = 1024;

    vec3 ambiant = vec3(1,1,1);
    vec3 diffuseLightColor = vec3(1); // white
    vec3 specularLightColor = vec3(1); // white

    vec3 diffuse = max(0, dot(L,N)) * diffuseLightColor;
    vec3 specular = pow(max(0, dot(R, V)), alpha) * specularLightColor;

    color = (0.1 * ambiant + 0.5 * diffuse + 0.001 * specular) * realColor;
    // color = normalToColor(N);
}
