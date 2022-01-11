#version 450 core
in vec4 fs_origin_norm;
in vec4 fs_norm;
out vec4 color0;
void main()
{
    float scale = 
    pow(max(dot(normalize(fs_norm), normalize(vec4(1.0, 1.0, -1.0, 0.0))), 0.0), 1.0)
    * 0.5 + 0.5;
    vec3 color = vec3(1.0, 1.0, 1.0);
    color0 = vec4(color * scale,1.0);
}