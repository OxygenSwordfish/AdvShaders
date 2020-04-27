#version 330 core
out vec4 FragColor;


in vec3 normES ;
in vec3 posES ;

in float heightTE;
in float heightCoTE;

in float visibilityTE;
in vec2 texCoordsTE;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
    float shininess;
};                                                                        


struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}; 

//uniform sampler2D texture1;
uniform DirLight dirLight;
uniform Material mat ;
uniform vec3 viewPos ;
uniform vec4 sky;
vec3 colour;


uniform mat4 lightSpaceMatrix;

uniform sampler2D shadowMap;

float gamma = 2.2;
float calcShadows(vec4 fragPosLightSpace);


uniform bool fogEnable, gammaCor, shadowEnable;

void main()
{   
	vec4 fragPosLightSpace = lightSpaceMatrix * vec4(posES,1.0);

	vec4 low = vec4(0.4,0.1,0.0,0.0); //brown
	vec4 mid = vec4(0.3,0.4,0.15,0.0); //green
	vec4 high = vec4(0.5,0.4,0.5,0.0); //grey



	float height = heightTE/heightCoTE;
	
	
	
	if (height > 0.5)
		colour = vec3(mix(mid,high, smoothstep(0.5,0.9,height)).rgb);
	else if (height < 0.2)
		colour =  vec3(mix(low,mid, smoothstep(0.0,0.2,height)).rgb);
	else 
		colour = mid.rgb;

	

    vec3 viewDir = normalize(viewPos - posES);
	vec3 norm = normalize(normES) ;
	vec3 ambient = dirLight.ambient * mat.ambient;     
    vec3 lightDir = normalize(-dirLight.direction);
    // diffuse shading
    float diff = max(dot(norm, dirLight.direction), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-dirLight.direction, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    // combine results
   
    vec3 diffuse  = dirLight.diffuse * (diff * mat.diffuse);
    vec3 specular = dirLight.specular * (spec * mat.specular);

	float shadow = calcShadows(fragPosLightSpace);
   if(shadowEnable)
		FragColor = vec4(ambient + (1.0-shadow)*(diffuse + specular)* colour,1.0f);
	else 
		FragColor = vec4((ambient + diffuse + specular)* colour, 1.0f);

	if(fogEnable)
		FragColor = mix(vec4(sky), FragColor, visibilityTE);

	//Gamma correction
	if(gammaCor)
		FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
	
}

float calcShadows(vec4 fragPosLightSpace)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowMap, projCoords.xy).r;

	float currentDepth = projCoords.z;

	float shadow = 0;

	float bias = 0.015; //Shadow bias
	//PCF

	vec2 texelSize = 1.0/textureSize(shadowMap,0);
	for (int i = -1; i<2; i++)
	{
		for(int j = -1; j<2; j++)
		{
			float pcf = texture(shadowMap, projCoords.xy + vec2(i,j)* texelSize).r;
			if(currentDepth - bias > pcf)
			shadow += 1;
		}
	}

	return shadow;
}