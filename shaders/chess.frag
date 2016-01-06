#version 130

uniform vec3 light;
uniform vec3 camera;
uniform int color;
uniform float shininess = 32;

in vec3 position;
in vec3 normal;

out vec3 fragColor;

void main()
{
    vec3 ambiant = vec3(1)/10.0;

    vec3 N = normalize(normal);

    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);

    vec3 L = normalize(light - position);
    diffuse += max(0, dot(L,N)) * vec3(1);

    vec3 V = normalize(camera - position);
    vec3 R = normalize(2 * dot(L,N) * N - L);
    specular += pow(max(0, dot(R,V)), shininess) * vec3(1);

    vec3 myColor;
    if(color == 0)
       myColor = vec3(1,0.5,0); // white guy
    else
       myColor = vec3(0,0.5,1); // black guy

    fragColor = (ambiant + diffuse + specular) * myColor;
    // fragColor = N; // (L+1)/2;
}
