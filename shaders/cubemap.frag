#version 130

uniform samplerCube cubemap;

in vec3 texcoord;
out vec4 color;

void main(void)
{
    color = texture(cubemap, texcoord);
    // color = vec4((texcoord+1)/2, 1);
}
