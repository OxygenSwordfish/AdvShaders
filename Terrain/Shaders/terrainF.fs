#version 330 core
out vec4 FragColor;


in vec3 gNormals ;
in vec3 gWorldPos_FS_in ;
in float heightG;
in float heightCoG;

in float visibilityG;



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

uniform sampler2D texture1;
uniform DirLight dirLight;
uniform Material mat ;
uniform vec3 viewPos ;
uniform vec4 sky;
vec3 colour;

void main()
{   
	vec4 low = vec4(0.7,0.4,0.0,0.0); //brown
	vec4 mid = vec4(0.3,0.4,0.15,0.0); //green
	vec4 high = vec4(0.5,0.4,0.5,0.0); //grey



	float height = heightG/heightCoG;
	
	
	
	if (height > 0.6)
		colour = vec3(mix(mid,high, smoothstep(0.5,0.9,height)).rgb);
	else if (height < 0.4)
		colour =  vec3(mix(low,mid, smoothstep(0.1,0.5,height)).rgb);
	else 
		colour = mid.rgb;
    vec3 viewDir = normalize(viewPos - gWorldPos_FS_in);
	vec3 norm = normalize(gNormals) ;
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
    FragColor = vec4((ambient + diffuse + specular)* colour,1.0f);
	FragColor = mix(vec4(sky), FragColor, visibilityG);
	
}

