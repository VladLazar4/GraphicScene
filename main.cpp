//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>


std::vector<const GLchar*> faces;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

int glWindowWidth = 2000;
int glWindowHeight = 1000;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;


glm::mat4 model;
glm::mat4 lightModel;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

glm::mat4 lightRotation;
GLfloat lightAngle = 0.0f;
glm::vec3 lightSun;
GLuint lightSunLoc;
glm::vec3 lightColorSun;
glm::vec3 lightColorFar;
glm::vec3 lightColorFelinar;
GLuint lightColorLoc;

gps::Camera myCamera(
	glm::vec3(-38.7511f, 5.55983f, 0.22628f),
	glm::vec3(-59.3648f, 6.58707f, -24.4626f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.3f;

bool pressedKeys[1024];

gps::Model3D myModel;
gps::Model3D dacia;
gps::Model3D elice;

gps::Shader myCustomShader;
gps::Shader depthMapShader;


GLuint shadowMapFBO;
GLuint depthMapTexture;

bool cameraAnimation = false;
int steps = 0;
float camera_coordinate_x = 0.0f;
float camera_coordinate_y = 0.0f;
float camera_coordinate_z = 0.0f;
float ct_camera_coordinate_x = 0.0f;
float ct_camera_coordinate_y = 0.0f;
float ct_camera_coordinate_z = 0.0f;
float camera_angle = 0.0f;
int currentStateCameraAnimation = -1;


bool fogON = true;
GLint foginitLoc;
GLfloat fogDensity = 0.005f;

float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch;
bool mouse = true;

bool faruriON = false;
bool felinarON = false;
glm::vec3 lightPosFelinar;
GLuint lightPosFelinarLoc;
glm::vec3 lightPosFar1;
GLuint lightPosFarLoc1;
glm::vec3 lightPosFar2;
GLuint lightPosFarLoc2;

float carMove = 0.0f;
bool carDirectionFront = true;
bool animationStart = false;
bool rotationStart = false;
float eliceRotation;

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);

	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	// set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (action == GLFW_PRESS) {
		switch (key) {

		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
			
		///LUMINA PUNCTIFORMA - FARURI SI FELINAR
		case GLFW_KEY_U:
			myCustomShader.useShaderProgram();
			faruriON = !faruriON;
			glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "faruriON"), faruriON);
			break;
		case GLFW_KEY_I:
			myCustomShader.useShaderProgram();
			felinarON = !felinarON;
			glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "felinarON"), felinarON);
			break;

		///MODURI DE VIZUALIZARE
		case GLFW_KEY_1: // solid/smooth
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			break;
		case GLFW_KEY_2: // wireframe
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			break;
		case GLFW_KEY_3: // poligonal (punctiform)
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			break;

		///ANIMATIE OBIECTE
		case GLFW_KEY_O:
			animationStart = !animationStart;
			break;
		
		///ANIMATIE DE PREZENTARE
		case GLFW_KEY_P:
			cameraAnimation = !cameraAnimation;
			break;

		///CEATA
		case GLFW_KEY_F: // start/stop ceata
			myCustomShader.useShaderProgram();
			fogON = !fogON;

			glUseProgram(myCustomShader.shaderProgram);
			foginitLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogON");
			glUniform1i(foginitLoc, fogON);

			glUseProgram(myCustomShader.shaderProgram);
			glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
			break;
		
		default:
			break;
		}
	}

	switch (key) {
	///LUMINA DIRECTIONALA - SOARE
	case GLFW_KEY_J:
		lightAngle += 1.0f;
		break;
	case GLFW_KEY_L:
		lightAngle -= 1.0f;
		break;

	case GLFW_KEY_EQUAL: // creste intensitate ceata
		fogDensity = fogDensity + 0.0001f < 1.0f ? fogDensity + 0.0001f : 1.0f;
		glUseProgram(myCustomShader.shaderProgram);
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
		break;

	case GLFW_KEY_MINUS: // scade intensitate ceata
		fogDensity = fogDensity - 0.0001f > 0.0f ? fogDensity - 0.0001f : 0.0f;
		glUseProgram(myCustomShader.shaderProgram);
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
		break;
	default:
		break;
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (mouse)
	{
		lastX = xpos;
		lastY = ypos;
		mouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.5f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
}

void processMovement()
{
	///W, A, S, D - movement
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightSunLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightSun));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightSunLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightSun));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightSunLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightSun));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightSunLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightSun));
	}


	///X, Z - UP/DOWN
	if (pressedKeys[GLFW_KEY_Z]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightSunLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightSun));
	}

	if (pressedKeys[GLFW_KEY_X]) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightSunLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightSun));
	}
}


void initObjects() {
	myModel.LoadModel("objects/overworld.obj");
	dacia.LoadModel("objects/dacia.obj");
	elice.LoadModel("objects/elice.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/startShader.vert", "shaders/startShader.frag");
	myCustomShader.useShaderProgram();

	depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
	depthMapShader.useShaderProgram();

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "Proiect", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.5, 0.5, 0.5, 1.0);

	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initUniforms() {

	myCustomShader.useShaderProgram();

	model = glm::mat4(2.0f);

	model = glm::translate(model, glm::vec3(7.0f, 0.0f, 0.0f));

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	///LUMINA
	lightSun = glm::vec3(-100.0f, 60.0f, 80.0f);
	lightRotation = glm::rotate(lightModel, glm::radians(lightAngle), glm::vec3(1.0f, 0.0f, 1.0f));

	lightSunLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightSun");
	glUniform3fv(lightSunLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightSun));

	lightColorSun = glm::vec3(1.0f, 1.0f, 1.0f);
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColorSun");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColorSun));

	lightColorFelinar = glm::vec3(1.0f, 0.64f, 0.0f);
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColorFelinar");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColorFelinar));
	
	lightColorFar = glm::vec3(1.0f, 0.64f, 0.5f);
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColorFar");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColorFar));

	lightPosFelinar = glm::vec3(-67.4618f, 4.35741f, -10.9f);
	lightPosFelinarLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPosFelinar");
	glUniform3fv(lightPosFelinarLoc, 1, glm::value_ptr(lightPosFelinar));
	
	lightPosFar1 = glm::vec3(-62.3935f - carMove, 1.22752f, -27.9523f - carMove);
	lightPosFarLoc1 = glGetUniformLocation(myCustomShader.shaderProgram, "lightPosFar1");
	glUniform3fv(lightPosFarLoc1, 1, glm::value_ptr(lightPosFar1));

	lightPosFar2 = glm::vec3(-64.1616f - carMove, 1.22752f, -26.1083f - carMove);
	lightPosFarLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightPosFar2");
	glUniform3fv(lightPosFarLoc2, 1, glm::value_ptr(lightPosFar2));
}

void initFBO() {
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawObjects(gps::Shader shader, bool depthPass) {
	shader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	dacia.Draw(shader);
	elice.Draw(shader);
	myModel.Draw(shader);
}

void moveFaruri() {
	lightColorFar = glm::vec3(1.0f, 0.64f, 0.5f);
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColorFar");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColorFar));

	lightPosFar1 = glm::vec3(-62.3935f - carMove, 1.22752f, -27.9523f - carMove);
	//lightPosFar = glm::vec3(-67.4618f, 4.35741f, -10.9f);
	lightPosFarLoc1 = glGetUniformLocation(myCustomShader.shaderProgram, "lightPosFar1");
	glUniform3fv(lightPosFarLoc1, 1, glm::value_ptr(lightPosFar1));

	lightPosFar2 = glm::vec3(-64.1616f - carMove, 1.22752f, -26.1083f - carMove);
	lightPosFarLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightPosFar2");
	glUniform3fv(lightPosFarLoc2, 1, glm::value_ptr(lightPosFar2));
}

glm::mat4 computeLightSpaceTrMatrix() {
	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightSun, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 1.0f, far_plane = 300.0f;
	glm::mat4 lightProjection = glm::ortho(-120.0f, 120.0f, -120.0f, 120.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

void renderScene() {

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	///ADAUGA UMBRE
	depthMapShader.useShaderProgram();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-carMove, 0, -carMove));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	dacia.Draw(depthMapShader);

	depthMapShader.useShaderProgram();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(32.5435f, 33.1106f, 19.8048f));
	model = glm::rotate(model, glm::radians(eliceRotation), glm::vec3(1, 0, 0));
	model = glm::translate(model, glm::vec3(-32.5435f, -33.1106f, -19.8048f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	elice.Draw(depthMapShader);

	depthMapShader.useShaderProgram();
	model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	myModel.Draw(depthMapShader);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	///ADAUGA OBIECTELE
	glViewport(0, 0, retina_width, retina_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	myCustomShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 0.0f, 1.0f));
	glUniform3fv(lightSunLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightSun));

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));


	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	myCustomShader.useShaderProgram();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-carMove, 0, -carMove));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	dacia.Draw(myCustomShader);

	myCustomShader.useShaderProgram();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(32.5435f, 33.1106f, 19.8048f));
	model = glm::rotate(model, glm::radians(eliceRotation), glm::vec3(1, 0, 0));
	model = glm::translate(model, glm::vec3(-32.5435f, -33.1106f, -19.8048f));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	elice.Draw(myCustomShader);

	myCustomShader.useShaderProgram();
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	moveFaruri();
	myModel.Draw(myCustomShader);
	mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);

	glfwTerminate();
}

void initFaces()
{
	faces.push_back("textures/skybox/right.png");
	faces.push_back("textures/skybox/left.png");
	faces.push_back("textures/skybox/top.png");
	faces.push_back("textures/skybox/bottom.png");
	faces.push_back("textures/skybox/front.png");
	faces.push_back("textures/skybox/back.png");
}

void initSkyBoxShader()
{
	mySkyBox.Load(faces);
	
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));
}

void cameraAnimationFunction() {
	if (cameraAnimation == true) {
		switch (currentStateCameraAnimation)
		{
		case -1:
			camera_coordinate_x = 0.0f;
			camera_coordinate_y = 0.0f;
			camera_coordinate_z = 0.0f;
			ct_camera_coordinate_x = -42.2354f;
			ct_camera_coordinate_y = 14.3752f;
			ct_camera_coordinate_z = 29.999f;
			camera_angle = 0.0f;
			steps = 0;
			currentStateCameraAnimation = 0;
			break;
		case 0:
			camera_coordinate_x -= 0.0025f;
			camera_coordinate_y -= 0.05f;
			steps++;
			if (steps > 90) {
				steps = 0;
				currentStateCameraAnimation = 1;
			}
			break;
		case 1:
			//camera_coordinate_y -= 0.15f;
			/*camera_coordinate_x -= 0.35f;*/
			//camera_coordinate_z -= 0.25f;
			camera_angle += 0.2f;
			steps++;
			if (steps > 700) {
				steps = 0;
				currentStateCameraAnimation = 2;
			}
			break; 
		case 2:
			camera_coordinate_x -= 0.03f;
			camera_coordinate_z -= 0.11f;
			camera_coordinate_y -= 0.017f;
			camera_angle -= 0.1f;
			steps++;
			if (steps > 1000) {
				steps = 0;
				currentStateCameraAnimation = 3;
			}
			break;
		case 3:
			camera_angle -= 0.2f;
			//camera_coordinate_y -= 0.005f;
			steps++;
			if (steps > 120) {
				steps = 0;
				currentStateCameraAnimation = 4;
			}
			break;
		case 4:
			//camera_angle -= 0.2f;
			camera_coordinate_y -= 0.005f;
			camera_coordinate_z -= 0.1f;
			camera_coordinate_x += 0.15f;
			steps++;
			if (steps > 200) {
				steps = 0;
				currentStateCameraAnimation = 5;
			}
			break;
		case 5:
			camera_coordinate_y -= 0.005f;
			camera_coordinate_z -= 0.2f;
			steps++;
			if (steps > 50) {
				steps = 0;
				currentStateCameraAnimation = -1;
				cameraAnimation = false;
			}
			break;
		default:
			currentStateCameraAnimation = 0;
			break;
		}
		myCamera.animation(camera_coordinate_x, camera_coordinate_y, camera_coordinate_z, ct_camera_coordinate_x, ct_camera_coordinate_y, ct_camera_coordinate_z, camera_angle);
	}
	else {
		currentStateCameraAnimation = -1;
	}
}

void objectsAnimationFunction() {
	if (animationStart == true) {
		if (carDirectionFront == true) {
			carMove += 0.01;
			if (carMove > 2.0f)
				carDirectionFront = false;
		}
		else {
			carMove -= 0.01;
			if (carMove < -2.0f)
				carDirectionFront = true;
		}

		if (eliceRotation < 360)
			eliceRotation += 0.5;
		else
			eliceRotation = 0;
	}
}

int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();

	initObjects();
	initShaders();
	initUniforms();

	initFBO();
	initFaces();
	initSkyBoxShader();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();
		cameraAnimationFunction();
		objectsAnimationFunction();
		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}