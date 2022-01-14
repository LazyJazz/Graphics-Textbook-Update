#version 450 core

in vec2 texcoord;

out vec4 color;

uniform sampler2D texture0;

void main()
{
	color = texture(texture0, texcoord) * 2.0 - 1.0;
}