#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormals;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoords;
out vec3 posVS; 
out vec3 normVS;

void main()
{
		texCoords = aTexCoords;
		normVS = aNormals ; 
		posVS = (model * vec4(aPos, 1.0)).xyz; 
		gl_Position = projection * view *vec4(aPos, 1.0);
}

