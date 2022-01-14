#version 450 core
layout (location = 0) in vec4 vs_pos;
layout (location = 1) in vec4 vs_color;
out vec4 fs_color;
uniform mat4 mat_proj;
void main()
{
    gl_Position = mat_proj*vs_pos;
    fs_color = vs_color;
}