#version 130

uniform vec3 light;

in vec3 position;
in vec3 normal;

out vec3 color;

void main()
{
    vec3 ambiant = vec3(1)/10.0;

    vec3 L = normalize(light - position);
    vec3 N = normalize(normal);

    vec3 diffuse = max(0, dot(L,N)) * vec3(1);

    color = (ambiant + diffuse) * vec3(1, 1, 0);
    // color = N; // (L+1)/2;
}
