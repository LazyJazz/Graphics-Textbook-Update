#version 450 core

layout (location = 0) in vec3 vs_pos;
layout (location = 1) in vec3 vs_norm;
layout (location = 2) in vec3 vs_color;

out vec4 fs_norm;
out vec4 fs_pos;
out vec3 fs_color;

uniform mat4 mat_proj;
uniform mat4 mat_trans;

void main()
{
    fs_color = vs_color;
    vec4 norm = vec4(vs_norm , 0.0);
    fs_norm = norm;
    fs_pos = vec4(vs_pos, 1.0);
    gl_Position = mat_proj*mat_trans*fs_pos;
}