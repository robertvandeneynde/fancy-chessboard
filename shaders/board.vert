#version 130

in vec3 vertexPosition;
in vec3 vertexNormal;

uniform int color;
uniform mat4 matrix;
uniform mat4 model;
uniform mat3 normalMatrix;

out vec3 position;
out vec3 normal;
out vec2 texCoord;

void main(void)
{
    position = vec3(model * vec4(vertexPosition, 1));
    normal = normalMatrix * vertexNormal;
    normal = vec3(0,0,1);
    texCoord = vec2(vertexPosition);
    gl_Position = matrix * vec4(vertexPosition, 1);
}
