#version 130

uniform vec3 light;
uniform int color;

in vec3 position;
in vec3 normal;

out vec3 fragColor;

void main()
{
    vec3 ambiant = vec3(1)/10.0;

    vec3 N = normalize(normal);

    vec3 diffuse = vec3(0);

    vec3 L = normalize(light - position);
    diffuse += max(0, dot(L,N)) * vec3(1);

    vec3 myColor;
    if(color == 0)
       myColor = vec3(1,0.5,0); // white guy
    else
       myColor = vec3(0,0.5,1); // black guy

    fragColor = (ambiant + diffuse) * myColor;
    // fragColor = N; // (L+1)/2;
}
