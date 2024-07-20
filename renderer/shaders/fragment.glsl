#version 330 core

in vec4 vertex_color;
in vec2 vertex_texcoord;

out vec4 out_color;

void main()
{
    out_color = vertex_color;
}