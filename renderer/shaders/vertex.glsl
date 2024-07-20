#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in vec2 in_texcoord;

out vec4 vertex_color;
out vec2 vertex_texcoord;

uniform mat4 u_transform;

void main()
{
    gl_Position = u_transform * vec4(in_position, 1.0);
    vertex_color = in_color;
    vertex_texcoord = in_texcoord;
}