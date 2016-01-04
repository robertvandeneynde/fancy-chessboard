#version 130

in vec3 vertexPosition;
uniform mat4 matrix;

void main()
{
    gl_Position = matrix * vec4(vertexPosition, 1);
}
