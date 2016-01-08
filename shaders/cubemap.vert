#version 130
in vec3 position;

uniform mat4 matrix;

out vec3 texcoord;

void main(void)
{
    gl_Position = matrix * vec4(position, 1);
    texcoord = position;
}
