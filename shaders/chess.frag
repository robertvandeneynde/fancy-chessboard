#version 130

uniform vec3 light;
uniform vec3 camera;
uniform int color;
uniform float shininess = 32;
uniform int lightingModel = 0; // PHONG
uniform float cookLambda = 0.4; // [0,1]
uniform float cookRoughness = 0.2;

const float Pi = 3.14159265358979323846;

in vec3 position;
in vec3 normal;

out vec3 fragColor;

void main()
{
    vec3 ambiant = vec3(1)/10.0;

    vec3 N = normalize(normal);

    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);

    vec3 lightColor = vec3(1);
    vec3 L = normalize(light - position);
    diffuse += max(0, dot(L,N)) * vec3(1);

    vec3 V = normalize(camera - position);
    vec3 R = normalize(2 * dot(L,N) * N - L);
    vec3 H = normalize(V + L);
    if(lightingModel < 2) {
        if(lightingModel == 0) {
            specular += pow(max(0, dot(R,V)), shininess) * lightColor;
        } else {
            specular += pow(max(0, dot(N,H)), shininess) * lightColor;
        }
    } else {
        // from wikipedia

        float VN = dot(V,N);
        float NL = dot(N,L);
        float F = pow(1+VN, cookLambda); // lambda is the relative indice of refrac
        // F can be approximated using Schlick's approximation, we won't use here.
        float NH = dot(N,H); // = cos angle between N and H
        float NH2 = NH * NH;
        float m2 = cookRoughness * cookRoughness;
        // float alphaAngle = acos(NH);
        float x = (1 - NH2) / (NH2 * m2);
        float D = exp(-x) / (Pi * m2 * NH2);
        float VH = dot(V,H);
        float G = min(1, min(2 * NH * VN / VH, 2 * NH * NL / VH));
        float kspec = D*F*G / (4 * (VN) * (NL));

        specular += kspec * lightColor;
    }

    vec3 myColor;
    if(color == 0)
       myColor = vec3(1,0.5,0); // white guy
    else
       myColor = vec3(0,0.5,1); // black guy

    fragColor = (ambiant + diffuse + specular) * myColor;
    // fragColor = N; // (L+1)/2;
}
