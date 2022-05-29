#version 460

layout (location = 0) out vec4 FragColor;

uniform vec4 Colour;

void main()
{
	FragColor = Colour;
}