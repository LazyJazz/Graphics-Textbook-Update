#version 450 core

layout (location = 0) in vec3 vs_pos;

uniform mat4 mat_trans;

void main()
{
    gl_Position = mat_trans*vec4(vs_pos, 1.0);
}