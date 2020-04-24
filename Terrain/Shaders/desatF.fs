#version 330 core

in vec2 texCoords;
uniform sampler2D scene;

out vec4 fragColor;


void main()
{
	vec4 orig = texture2D(scene, texCoords);

		float avg =(orig.x + orig.y + orig.z)/3;
		fragColor = vec4(vec3(avg),1.0);
	
}