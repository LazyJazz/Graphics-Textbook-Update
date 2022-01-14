#version 450 core

layout (location = 0) in vec2 coord;

out vec2 texcoord;

void main()
{
	texcoord = (coord + 1.0) * 0.5;
	gl_Position = vec4(coord, 0.0, 1.0);
}

