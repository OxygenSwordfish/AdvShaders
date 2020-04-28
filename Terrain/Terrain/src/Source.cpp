#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Shader.h"
#include "Camera.h"

#include <Model.h>
#include "Terrain.h"

#include<string>
#include <iostream>
#include <numeric>
#include <time.h>

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;
glm::vec3 dirLightPos(20.f, 0.6f, 0.2f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const * path);



void setVAO(vector <float> vertices);
void setFBOcolour();
void setFBOdepth();
void renderQuad();
void setShadowBuffer();
// camera
Camera camera(glm::vec3(260, 50, 300));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//arrays
unsigned int VBO, VAO, FBO, depthFBO, quadVAO, quadVBO, shadowMapFBO;
unsigned int textureColourBuffer;
unsigned int textureDepthBuffer;
unsigned int shadowMap;

void renderTerrain(vector<float> vertices);
//Update buffer object as new terrain generates
void updateBuffer(vector<float> vertices);

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec4 sky = glm::vec4(0.2, 0.4, 0.8, 1.0);

//Quickly debug depth without having to change multiple lines
bool depthDebug = false;

int postProcs = 0;

float terrSeed = 0.0f;

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

//Toggles for effects
bool rPress, gPress, tPress, fPress, yPress, hPress = false;

bool fogEnable, gammaCor, shadowEnable, showDepth, blinn = false;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "IMAT3907", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// simple vertex and fragment shader - add your own tess and geo shader
	//Shader shader("..\\shaders\\PNVert.vs", "..\\shaders\\phongDirFrag.fs", "..\\shaders\\tessGeo.gs", "..\\shaders\\PNtessC.tcs", "..\\shaders\\PNtessE.tes");
	Shader terrainShader("..\\shaders\\terrainV.vs", "..\\shaders\\terrainF.fs", nullptr, "..\\shaders\\terrainTC.tcs", "..\\shaders\\terrainTE.tes");
	//unsigned int heightMap = loadTexture("..\\resources\\hm1.jpg");
	//terrainShader.setInt("heightMap", 0);
	Shader postProcessor("..\\shaders\\postProcV.vs", "..\\shaders\\postProcF.fs");
	postProcessor.setBool("depthDebug", depthDebug);

	Shader desat("..\\shaders\\postProcV.vs", "..\\shaders\\desatF.fs");
	Shader invert("..\\shaders\\postProcV.vs", "..\\shaders\\invertF.fs");
	Shader nightVision("..\\shaders\\postProcV.vs", "..\\shaders\\nightVis.fs");
	unsigned int overlayNoise = loadTexture("..\\resources\\overlayNoise.png");
	nightVision.setInt("overlayNoise", 1);

	
	Shader glitch("..\\shaders\\postProcV.vs", "..\\shaders\\glitch.fs");

	Shader shadows("..\\shaders\\shadowsV.vs", "..\\shaders\\shadowsF.fs");

	//Terrain Constructor ; number of grids in width, number of grids in height, gridSize
	Terrain terrain;
	std::vector<float> vertices = terrain.getVertices();
	setVAO(vertices);
	setFBOdepth();
	setFBOcolour();
	setShadowBuffer();
	
	srand(time(NULL));
	terrSeed = rand();

	terrainShader.use();
	terrainShader.setFloat("seed", terrSeed);
	terrainShader.setInt("shadowMap", 0);
	
	

	while (!glfwWindowShouldClose(window))
	{

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(sky.r, sky.g, sky.b, sky.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1200.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		
		glm::mat4 lightProjection, lightView, lightSpaceMatrix;
		float nearPlane = 1.0f, farPlane = 1000.f;

		terrainShader.use();
		terrainShader.setMat4("projection", projection);
		terrainShader.setMat4("view", view);


		terrainShader.setMat4("model", model);
		terrainShader.setVec3("viewPos", camera.Position);
		terrainShader.setVec3("eyePos", camera.Position);

		//light properties
		terrainShader.setVec3("dirLight.direction", dirLightPos);
		terrainShader.setVec3("dirLight.ambient", 0.5f, 0.5f, 0.5f);
		terrainShader.setVec3("dirLight.diffuse", 0.55f, 0.55f, 0.55f);
		terrainShader.setVec3("dirLight.specular", 0.6f, 0.6f, 0.6f);
		//material properties
		terrainShader.setVec3("mat.ambient", 1.0, 1.0, 1.0);
		terrainShader.setVec3("mat.diffuse", 0.396, 0.741, 0.691);
		terrainShader.setVec3("mat.specular", 0.297f, 0.308f, 0.306f);
		terrainShader.setFloat("mat.shininess", 0.9f);

		lightProjection = glm::ortho(-250.0f, 250.0f, -250.0f, 250.0f, nearPlane, farPlane);
		lightView = glm::lookAt(dirLightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		terrainShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		terrainShader.setVec4("sky", sky);

		terrainShader.setBool("shadowEnable", shadowEnable);
		terrainShader.setBool("fogEnable", fogEnable);
		terrainShader.setBool("gammaCor", gammaCor);
		terrainShader.setBool("blinn", blinn);
		shadows.use();
		shadows.setMat4("lightSpaceMatrix",lightSpaceMatrix);

		postProcessor.use();
		postProcessor.setBool("depthDebug", showDepth);
		if(showDepth)
		{
			glBindFramebuffer(GL_FRAMEBUFFER,depthFBO);
			terrainShader.use();
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderTerrain(vertices);
			
			
			glBindFramebuffer(GL_FRAMEBUFFER,0);
			glDisable(GL_DEPTH_TEST);
			postProcessor.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureDepthBuffer);
			renderQuad();
		}
		else
		{
			postProcessor.setBool("depthDebug", false);
			//update generate new terrain if camera moves enough. Terrain origin not updating but rest works
			/*if (terrain.bounds(camera.Position.x, camera.Position.z, 100.f) == true)
			{
				terrSeed = rand();
				terrainShader.setFloat("seed",terrSeed);
				terrain.makeVertices(&terrain.getVertices(), camera.Position.x, camera.Position.z);
				vertices = terrain.getVertices();
				updateBuffer(vertices);
			}*/

			if (shadowEnable) {
				//Framebuffer first pass - Shadows
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glEnable(GL_DEPTH_TEST);
			glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			shadows.use();

			renderTerrain(vertices);
			
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			terrainShader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowMap);
			renderTerrain(vertices);
			}

			else {
				glBindFramebuffer(GL_FRAMEBUFFER, FBO);

				terrainShader.use();
				terrainShader.setFloat("seed", terrSeed);
				glEnable(GL_DEPTH_TEST);


				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
				renderTerrain(vertices);

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				switch (postProcs) {
				case 0:
					postProcessor.use();
					break;
				case 1:
					desat.use();
					break;
				case 2: invert.use();
					break;
				case 3: nightVision.use();
					nightVision.setFloat("time", glfwGetTime());
					break;
				case 4: glitch.use();
					glitch.setFloat("time", glfwGetTime());
					break;
				}
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glDisable(GL_DEPTH_TEST);

				
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textureDepthBuffer);
				glBindTexture(GL_TEXTURE_2D, textureColourBuffer);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				renderQuad();
			}
				



			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			//renderTerrain(vertices);
		}
		

		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
			camera.printCameraCoords();

		if ((glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) && rPress == false)
		{
			terrSeed = rand();
			terrainShader.setFloat("seed", terrSeed);
			rPress = true;
		}
		

		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
			rPress = false;


		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		postProcs = 0;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		postProcs = 1;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		postProcs = 2;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		postProcs = 3;
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		postProcs = 4;


	
	if ((glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) && gPress == false)
	{
		gammaCor = !gammaCor;
		gPress = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) && tPress == false)
	{
		shadowEnable = !shadowEnable;
		tPress = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) && fPress == false)
	{
		fogEnable = !fogEnable;
		fPress = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) && yPress == false)
	{
		showDepth = !showDepth;
		yPress = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) && hPress == false)
	{
		blinn = !blinn;
		hPress = true;
	}


	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
		gPress = false;
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
		tPress = false;
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
		fPress = false;
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE)
		yPress = false;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE)
		hPress = false;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
		std::cout << "Loaded texture at path: " << path << " width " << width << " id " << textureID << std::endl;

	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);

	}

	return textureID;
}


void setVAO(vector <float> vertices) {

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(GLfloat)), vertices.data(), GL_STATIC_DRAW);

	//xyz
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//texture
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//normals
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);



	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void updateBuffer(vector<float> vertices)
{
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(GLfloat)), vertices.data(), GL_STATIC_DRAW);

	//xyz
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//texture
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//normals
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);



	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void setFBOcolour()
{
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &textureColourBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColourBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, textureColourBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBuffer, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
}

void setFBOdepth()
{
	glGenFramebuffers(1, &depthFBO);
	glGenTextures(1, &textureDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, textureDepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepthBuffer, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void setShadowBuffer() 
{
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f, };

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
void renderTerrain(vector<float> vertices)
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_PATCHES, 0, vertices.size() / 3);
	//glBindVertexArray(0);
}

