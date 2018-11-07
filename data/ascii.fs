#version 330 core

in vec2 texCoord;
out vec3 colour;

uniform sampler2D fontText;

void main()
{
	colour = texture(fontText, texCoord).rgb;
}

