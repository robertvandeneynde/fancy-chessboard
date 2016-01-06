#version 130
uniform vec3 P[4];
uniform mat4 matrix;

in float t; // from 0 to 1

out vec3 vertexColor;

uniform int degree;

void main()
{
    float u = 1 - t;
    vec3 vertexPosition;
    if(degree == 4)
        vertexPosition = u*u*u * P[0] + 3*u*u*t * P[1] + 3*u*t*t * P[2] + t*t*t * P[3];
    else
        vertexPosition = u*u * P[0] + 2*u*t * P[1] + t*t * P[2];
    gl_Position = matrix * vec4(vertexPosition, 1);
    vertexColor = vec3(1,0,0) * t + u * vec3(0,1,0);
}
