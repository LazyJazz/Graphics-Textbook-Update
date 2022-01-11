#version 450 core
in vec2 fs_texcoord;
out vec4 color;
uniform sampler2D texture0;
void main()
{
    color = texture(texture0, fs_texcoord);
}