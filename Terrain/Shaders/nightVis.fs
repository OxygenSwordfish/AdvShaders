#version 330 core

// based on code source at: tobiasbu.wordpress.com/2016/01/16/opengl-night-vision-shader/

in vec2 texCoords;
uniform sampler2D scene;
uniform sampler2D overlayNoise;
uniform float time;

out vec4 fragColor;




void main()
{
		
	vec2 uv;
	uv.x = 0.35*sin(time * 50.0);
	uv.y = 0.35*cos(time * 50.0);

	vec3 noise = texture(overlayNoise, texCoords.xy + uv).rgb;

	vec3 colour = texture(scene, texCoords.xy +  (noise.xy*0.005)).rgb;

	const vec3 luminosity = vec3(0.3,0.59,0.11);

	float intensity = dot(luminosity,colour);

	intensity = clamp(0.5 * (intensity - 0.5) + 0.5, 0.0, 1.0);

	float verdancy = clamp(intensity/0.59,0.0,1.0);
	
	vec3 greenFilter = vec3(0,verdancy,0);

	fragColor = vec4((colour + (noise*0.2)) * greenFilter, 0.8);
}

