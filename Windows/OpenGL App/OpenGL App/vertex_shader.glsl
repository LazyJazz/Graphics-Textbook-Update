#version 450 core

layout (location = 0) in vec3 vs_pos;
layout (location = 1) in vec3 vs_norm;
layout (location = 2) in vec3 vs_color;
layout (location = 3) in int vs_flag;

out vec3 fs_norm;
out vec3 fs_pos;
out vec3 fs_color;
out int fs_flag;

uniform mat4 mat_proj;
uniform mat4 mat_trans;

void main()
{
    fs_flag = vs_flag;
    fs_color = vs_color;
    vec4 norm = vec4(vs_norm , 0.0);
    fs_norm = vs_norm;
    fs_pos = vs_pos;
    gl_Position = mat_proj*mat_trans*vec4(fs_pos, 1.0);
}