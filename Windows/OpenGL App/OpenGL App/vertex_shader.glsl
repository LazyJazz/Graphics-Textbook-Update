#version 450 core

layout (location = 0) in vec3 vs_pos;

out vec4 fs_origin_norm;
out vec4 fs_norm;
out vec4 fs_pos;

uniform mat4 mat_proj;
uniform mat4 mat_trans;

void main()
{
    vec4 norm = vec4(vs_pos , 0.0);
    fs_origin_norm = norm;
    norm = mat_trans*norm;
    fs_norm = norm;
    fs_pos = mat_trans*vec4(vs_pos, 1.0);
    gl_Position = mat_proj*fs_pos;
}