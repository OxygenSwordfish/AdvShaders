#version 330 core

in vec2 texCoords;
uniform sampler2D scene;

out vec4 fragColor;


void main()
{
	vec4 orig = texture2D(scene, texCoords);

	fragColor = 1 - orig;
	
}