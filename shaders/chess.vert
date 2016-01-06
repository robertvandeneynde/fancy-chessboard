#version 130

in vec3 vertexPosition;
in vec3 vertexNormal;
uniform mat4 matrix;
uniform mat4 model;;
uniform mat3 normalMatrix;
uniform int color;

out vec3 position;
out vec3 normal;

void main(void)
{
    position = vec3(model * vec4(vertexPosition, 1));
    normal = normalMatrix * vertexNormal;
    gl_Position = matrix * vec4(vertexPosition, 1);
}
