#version 330 core

in vec2 texCoords;
uniform sampler2D scene;
uniform float time;

out vec4 fragColor;

const int max = 300;
const int min = 56;

void main()
{
	
	float alpha = clamp((cos(time)+1)/2,0.0,1.0);
	float alpha2 = clamp(sin(time)+cos(time),0.0,1.0);
	
	vec2 coords = vec2(floor(texCoords.x * mix(max, min, alpha))/mix(max, min, alpha),
					floor(texCoords.y * mix(max, min, alpha))/mix(max, min, alpha));

	vec4 colour1 = texture2D(scene,coords);
	float redshift = texture2D(scene , texCoords + (0.001 * (1-alpha))).r;
	float greenshift = texture2D(scene , texCoords + (0.001 * (1-alpha))).g;
	vec4 colour2 = vec4(redshift,greenshift, texture2D(scene,texCoords).b, 1.0);

	fragColor = mix(texture2D(scene,texCoords),mix(colour1,colour2,alpha2),1);


	

	
	
}