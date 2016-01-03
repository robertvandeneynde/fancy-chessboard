#version 130
in vec3 vertexPosition;
in vec3 vertexColor;
in vec2 vertexCoord;
in vec3 vertexNormal;

out vec3 outColor;
out vec3 normal;
out vec2 outCoord;
out vec3 position;
out vec3 outNormal;

uniform mat4 matrix;
uniform mat4 normMatrix;

void main()
{
    outColor = vertexColor;
    outCoord = vertexCoord;
    outNormal = vertexNormal;
    position = vertexPosition;
    gl_Position = matrix * vec4(vertexPosition, 1.0); // proj * model * pos
    normal = mat3(normMatrix) * vertexNormal;
}
