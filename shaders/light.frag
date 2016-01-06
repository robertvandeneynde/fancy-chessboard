#version 130

out vec3 fragColor;

uniform vec3 color = vec3(1,1,1);

void main() {
    fragColor = color;
}
