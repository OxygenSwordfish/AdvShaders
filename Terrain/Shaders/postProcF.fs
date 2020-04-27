#version 330 core

in vec2 texCoords;
uniform sampler2D scene;
float LineariseDepth(float depth);

out vec4 fragColor;
const float near_plane = 1;
const float far_plane = 250;

uniform bool depthDebug;


void main()
{
	float depth = texture(scene, texCoords).r;
	vec4 orig = texture2D(scene, texCoords);
	if (depthDebug) 		
		fragColor = vec4(vec3(LineariseDepth(depth)/far_plane), 1.0);
	else 
		fragColor = orig;
	
}


float LineariseDepth(float depth)
{
	float z = depth * 2.0 - 1;
	return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z *(far_plane - near_plane));
}