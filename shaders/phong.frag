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

uniform mat4 normMatrix;

out vec3 color;

void main()
{
    vec3 constColor = vec3(0,0,0.5);
    vec3 bumpColor = texture2D(bump, outCoord).rgb;
    // bumpColor = 2 * bumpColor - 1; // from [0,1] to [-1,1]
    vec3 realColor = texture2D(diag, outCoord).rgb;
    // realColor = vec3(1,0,0); // outColor;
    vec3 L = normalize(light - position);
    vec3 N = mat3(normMatrix) * normalize(bumpColor); // take normal from bump map
    // N = normalize(normal); // simply take normal

    vec3 V = camera - position;
    vec3 R = normalize(2 * dot(L,N) * N - L);

    float alpha = 64;
    vec3 factors = vec3(0.0, 1, 0.0);

    vec3 ambiant = vec3(1,1,1);
    vec3 diffuseLightColor = vec3(1,1,1); // white
    vec3 specularLightColor = vec3(1,1,1); // white

    vec3 diffuse = max(0, dot(L,N)) * diffuseLightColor;
    vec3 specular = pow(max(0, dot(R, V)), alpha) * specularLightColor;

    color = mat3(ambiant, diffuse, specular) * factors;
    color *= constColor;
    color = outColor;
    // color = constColor;
}
