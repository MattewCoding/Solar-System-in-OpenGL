// ----------------------------------------------------------------------------
// main.cpp
//
//  Created on: 24 Jul 2020
//      Author: Kiwon Um
//        Mail: kiwon.um@telecom-paris.fr
//
// Description: IGR201 Practical; OpenGL and Shaders (DO NOT distribute!)
//
// Copyright 2020-2024 Kiwon Um
//
// The copyright to the computer program(s) herein is the property of Kiwon Um,
// Telecom Paris, France. The program(s) may be used and/or copied only with
// the written permission of Kiwon Um or in accordance with the terms and
// conditions stipulated in the agreement/contract under which the program(s)
// have been supplied.
// ----------------------------------------------------------------------------

#define _USE_MATH_DEFINES

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "camera.h"
#include "mesh.h"
#include "meshUtility.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <dep/glm/glm.hpp>
#include <dep/glm/ext.hpp>

#include <cmath>
#include <ctime> 
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseMotionCallback(GLFWwindow* window, double xpos, double ypos);

// constants
const static float kSizeSun = 1;
const static float kSizeEarth = kSizeSun * 0.5;
const static float kSizeMoon = kSizeEarth * 0.5;
const static std::vector<float> planetSizes = { 0.5f, 0.19f, 0.47f, 0.27f, 5.6f, 4.72f, 2.0f, 1.94f, 0.09f};

const static float kRadOrbitEarth = 10;
const static float kRadOrbitVenus= 5;
const static float kRadOrbitMoon = 4;
const static std::vector<float> orbitRadii = { 10.0, 3.87, 7.23, 15.24, 52.03, 95.72, 191.64, 301.80, 394.81 };

//const static float earthRotationSpeed = 60.*500.;
const static float slowdownRatio = 500;

// planetRotItself scaled such that the planets rotate in a reasonable time.
// Scaled such that rot_duration(venus) = -rot_duration(earth)
const static std::vector<float> planetRotItself = { 1.0, 1.47, -1.0, 1.22, 1.0, 1.0, 1.0, 0.99, 1.0, 0.94, };
const static std::vector<float> planetRotSun = { 1.0, 0.24, 0.62, 1.88, 11.86, 29.43, 83.76, 163.75, 247.97 };

const static std::vector<float> axialTilt = { 0.41, 0.0, 3.1, 0.44, 0.05, 0.47, 1.71, 0.49, 2.09 };
static std::vector<float> axialTiltCos = {};
static std::vector<float> axialTiltSin = {};
static std::vector<float> orbitInclCos = {};
static std::vector<float> orbitInclSin = {};

static std::vector<float> orbitIncl = { 0.0, 0.12, 0.06, 0.03, 0.02, 0.04, 0.01, 0.03, 0.3 };

const static float x_sun = 0, y_sun = 0, z_sun = 0;
const static float x_venus = x_sun + kRadOrbitVenus, x_earth = x_sun + kRadOrbitEarth, x_moon = x_earth + kRadOrbitMoon;

const std::string backoutPath = "../../../";

int nbPlanetsToRender = 9;

// Window parameters
GLFWwindow* g_window = nullptr;

// GPU objects
// A GPU program contains at least a vertex shader and a fragment shader
GLuint g_program = 0;

// Basic camera model
Camera g_camera;

// Toy mesh for a sphere
std::shared_ptr<Mesh> sphereMesh;
std::shared_ptr<Mesh> sunSphere, venusSphere, earthSphere, moonSphere;
std::vector < std::shared_ptr<Mesh> > planets;

// Translation matrixes
glm::mat4 g_sun, g_venus, g_earth, g_moon;

// Rotation matrixes
//glm::mat4 earth_rot{ glm::mat4(1.) }, moon_rot{ glm::mat4(1.) };

// Texture vars
GLuint g_sunTexID, g_moonTexID;
std::vector <GLuint> texIDs;

// Updating vars
float fps = 60, lastUpdateTime = 0, fpsSkip = 120.0 / fps;

// Mouse vars
bool rightMousePressed = false, leftMousePressed = false, invertedMouseControls = false;
double lastX, lastY;

void printMat4(glm::mat4 a)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			std::cout << a[i][j] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

GLuint loadTextureFromFileToGPU(const std::string& filename) {
	// Loading the image in CPU memory using stb_image
	//std::cout << "Loading " << backoutPath + filename << "..." << std::endl;

	int width, height, numComponents;
	unsigned char* data = stbi_load((backoutPath + filename).c_str(), &width, &height, &numComponents, 0);

	if (!data) {
		std::cerr << "Failed to load texture: " << filename << std::endl;
		return 0; // Return 0 to indicate failure
	}

	GLuint texID; // OpenGL texture identifier
	glGenTextures(1, &texID); // generate an OpenGL texture container
	glBindTexture(GL_TEXTURE_2D, texID); // activate the texture

	// Setup the texture filtering option and repeat mode; check www.opengl.org for details.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Fill the GPU texture with the data stored in the CPU image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	// Free useless CPU memory
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture
	//std::cout << filename << " loaded." << std::endl;

	return texID;
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow* window, int width, int height) {
	g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
	glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_W) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else if (key == GLFW_KEY_F) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(window, true); // Closes the application if the escape key is pressed
		}

		else if (key == GLFW_KEY_Y) {
			invertedMouseControls = !invertedMouseControls;
		}

		else if (key == GLFW_KEY_Z) {
			g_camera.setFar(g_camera.getFar() * 9.0f / 10.0f);
		}
		else if (key == GLFW_KEY_X) {
			g_camera.setFar(g_camera.getFar() * 10.0f / 9.0f);
		}
		else if (key == GLFW_KEY_C) {
			g_camera.setPosition(glm::vec3(0.0f, 10.0f, 30.0f));
			g_camera.setCenter(ZERO_VECTOR);
		}
		else if (key == GLFW_KEY_T && nbPlanetsToRender < 9) {
			nbPlanetsToRender++;
		}
		else if (key == GLFW_KEY_G && nbPlanetsToRender > 1) {
			nbPlanetsToRender--;
		}
	}
}

glm::vec3 computeCameraMovement(Camera camera, double xRot, double yRot) {
	glm::vec3 camPos = camera.getPosition();
	glm::vec3 camCenter = camera.getCenter();

	double xRotRad = -xRot;
	double yRotRad = -yRot;

	glm::vec4 newCamPos =
		MeshUtility::translate(camCenter) *
		MeshUtility::rotateAroundAxis(Y_ROTATION_VECTOR, xRotRad) *
		MeshUtility::rotateAroundAxis(glm::vec3(1.0, 0.0, -camPos.x / camPos.z), yRotRad) *
		MeshUtility::translate(-camCenter) *
		glm::vec4{ camPos, 1.0 };

	//std::cout << newCamPos.x << ", " << newCamPos.y << ", " << newCamPos.z << std::endl;

	return glm::vec3(newCamPos);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			rightMousePressed = true;
			// Store initial mouse position
			glfwGetCursorPos(window, &lastX, &lastY);
		}
		else if (action == GLFW_RELEASE) {
			rightMousePressed = false;
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			leftMousePressed = true;
			// Store initial mouse position
			glfwGetCursorPos(window, &lastX, &lastY);
		}
		else if (action == GLFW_RELEASE) {
			leftMousePressed = false;
		}
	}
}

void mouseMotionCallback(GLFWwindow* window, double xpos, double ypos) {
	double deltaX = xpos - lastX;
	double deltaY = ypos - lastY;
	lastX = xpos;
	lastY = ypos;
	if (rightMousePressed) {
		glm::vec3 lookVector = g_camera.getPosition() - g_camera.getCenter();

		glm::vec3 perpXYVector = glm::vec3(lookVector.x, 0.0f, lookVector.z);
		float moveScaling = glm::length(perpXYVector) * 30.0;

		double dx = -deltaX * lookVector.z / moveScaling;
		double dz = deltaX * lookVector.x / moveScaling;

		g_camera.setPosition(g_camera.getPosition() + glm::vec3(dx, deltaY / 30.0, dz));
		g_camera.setCenter(g_camera.getCenter() + glm::vec3(dx, deltaY / 30.0, dz));
	}
	if (leftMousePressed) {
		if (invertedMouseControls)
		{
			deltaX *= -1;
			deltaY *= -1;
		}
		g_camera.setPosition(computeCameraMovement(g_camera, deltaX / 200.0, deltaY / 400.0));
	}
}

double scrollScaling = 11.0 / 10.0;
void mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	if (yOffset == 0) return;

	if (yOffset < 0)
	{
		yOffset = -yOffset * scrollScaling;
	}
	else
	{
		yOffset = abs(1.0 / (yOffset * scrollScaling));
	}

	glm::vec3 newCamPos = g_camera.getPosition() - g_camera.getCenter();
	newCamPos *= (yOffset);
	newCamPos += g_camera.getCenter();
	g_camera.setPosition(newCamPos);
}


void errorCallback(int error, const char* desc) {
	std::cout << "Error " << error << ": " << desc << std::endl;
}

void initGLFW() {
	glfwSetErrorCallback(errorCallback);

	// Initialize GLFW, the library responsible for window management
	if (!glfwInit()) {
		std::cerr << "ERROR: Failed to init GLFW" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// Before creating the window, set some option flags
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	// Create the window
	g_window = glfwCreateWindow(
		1536, 864,
		"Interactive 3D Applications (OpenGL) - Simple Solar System",
		nullptr, nullptr);
	if (!g_window) {
		std::cerr << "ERROR: Failed to open window" << std::endl;
		glfwTerminate();
		std::exit(EXIT_FAILURE);
	}

	// Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
	glfwMakeContextCurrent(g_window);
	glfwSetWindowSizeCallback(g_window, windowSizeCallback);

	glfwSetKeyCallback(g_window, keyCallback);
	glfwSetMouseButtonCallback(g_window, mouseButtonCallback);
	glfwSetCursorPosCallback(g_window, mouseMotionCallback);
	glfwSetScrollCallback(g_window, mouseScrollCallback);
}

void initOpenGL() {
	// Load extensions for modern OpenGL
	if (!gladLoadGL(glfwGetProcAddress)) {
		std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
		glfwTerminate();
		std::exit(EXIT_FAILURE);
	}

	glCullFace(GL_BACK); // Specifies the faces to cull (here the ones pointing away from the camera)
	glEnable(GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
	glDepthFunc(GL_LESS);   // Specify the depth test for the z-buffer
	glEnable(GL_DEPTH_TEST);      // Enable the z-buffer test in the rasterization
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // specify the background color, used any time the framebuffer is cleared
}

// Loads the content of an ASCII file in a standard C++ string
std::string file2String(const std::string& filename) {
	std::ifstream t(filename.c_str());

	std::stringstream buffer;
	buffer << t.rdbuf();

	return buffer.str();
}

// Loads and compile a shader, before attaching it to a program
void loadShader(GLuint program, GLenum type, const std::string& shaderFilename) {
	GLuint shader = glCreateShader(type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
	std::string shaderSourceString = file2String(shaderFilename); // Loads the shader source from a file to a C++ string
	const GLchar* shaderSource = (const GLchar*)shaderSourceString.c_str(); // Interface the C++ string through a C pointer
	glShaderSource(shader, 1, &shaderSource, NULL); // load the vertex shader code
	glCompileShader(shader);
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
	}
	glAttachShader(program, shader);
	glDeleteShader(shader);
}

void initGPUprogram() {
	g_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline

	// The main GPU program handling streams of polygons
	loadShader(g_program, GL_VERTEX_SHADER, backoutPath + "vertexShader.glsl");
	loadShader(g_program, GL_FRAGMENT_SHADER, backoutPath + "fragmentShader.glsl");
	glLinkProgram(g_program);

	const std::vector<std::string> planetPaths = { "earth","mercury", "venus",  "mars", "jupiter", "saturn", "uranus", "neptune", "pluto" };
	for each (std::string planetPath in planetPaths)
	{
		texIDs.push_back(loadTextureFromFileToGPU("media/" + planetPath + ".jpg"));
	}

	g_moonTexID = loadTextureFromFileToGPU("media/moon.jpg");
	g_sunTexID = loadTextureFromFileToGPU("media/sun.jpg");

	glUniform1i(glGetUniformLocation(g_program, "material.albedoTex"), 0);
	glUseProgram(g_program);
}

/*
* @brief Set up a 4x4 matrix to move and size the body.
* 
* @param size The size of the body
* @param x The x-coordinate of the body
* @param y The y-coordinate of the body
* @param z The z-coordinate of the body
*/
glm::mat4 setUpMatrix(float size, float x, float y, float z)
{
	return glm::scale(glm::translate(glm::mat4{ 1.0f }, glm::vec3{ x, y, z }), glm::vec3{ size });
}

// Define your mesh(es) in the CPU memory
void initCPUgeometry() {
	g_sun = setUpMatrix(kSizeSun, x_sun, y_sun, z_sun);
	//g_venus = setUpMatrix(kSizeEarth, x_venus, y_sun, z_sun);
	//g_earth = setUpMatrix(kSizeEarth, x_earth, y_sun, z_sun);
	g_moon = setUpMatrix(kSizeMoon, x_moon, y_sun, z_sun);

	// Reminder: this is here and not earlier because the program needs to init the shaders 'n stuff.
	sphereMesh = Mesh::genSphere(x_sun, y_sun, z_sun);

	sunSphere = std::make_shared<Mesh>(*sphereMesh);
	//venusSphere = std::make_shared<Mesh>(*sphereMesh);
	//earthSphere = std::make_shared<Mesh>(*sphereMesh);
	moonSphere = std::make_shared<Mesh>(*sphereMesh);

	// Workaround because appearently calling this method in genSphere()'s init()
	// Doesn't actually work	
	sunSphere->defineRenderMethod();
	moonSphere->defineRenderMethod();

	sunSphere->move(g_sun);
	moonSphere->move(g_moon);
	//moonSphere->setupPlanet(0);

	sunSphere->setupSun();

	// You will literally never see the difference if this is uncommented because the sun is a solid color
	//sunSphere->rotateAround(sunSphere.get(), X_ROTATION_VECTOR, -M_PI / 2);
	
	/*earthSphere = std::make_shared<Mesh>(*sphereMesh);
	earthSphere->defineRenderMethod();
	earthSphere->move(setUpMatrix(kSizeSun * planetSizes[2], x_sun + orbitRadii[2], y_sun, z_sun));
	earthSphere->setupPlanet(-axialTilt[2]);*/

	std::srand(static_cast<unsigned int>(std::time(0)));
	for (int i = 0; i < 9; i++)
	{
		orbitIncl[i] *= 2.0;
		double orbitProgress = std::rand() % 135 / 180.0 * M_PI;
		std::shared_ptr<Mesh> planet = std::make_shared<Mesh>(*sphereMesh);
		planet->defineRenderMethod();
		planet->move(setUpMatrix(kSizeSun * planetSizes[i], x_sun + orbitRadii[i], y_sun, z_sun));
		planet->setupPlanet(-axialTilt[i], orbitProgress, orbitIncl[i]);
		if (i == 0) moonSphere->setupPlanet(0, orbitProgress, 0); // moon needs to align with Earth
		planets.push_back(planet);

		axialTiltCos.push_back(cos(axialTilt[i]));
		axialTiltSin.push_back(sin(axialTilt[i]));

		orbitInclCos.push_back(cos(orbitIncl[i]));
		orbitInclSin.push_back(sin(orbitIncl[i]));
	}
}

void initCamera() {
	int width, height;
	glfwGetWindowSize(g_window, &width, &height);
	g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

	// A little bit up high and far away
	g_camera.setPosition(glm::vec3(0.0, 10.0, 30.0));
	g_camera.setNear(g_camera.getNear());
	g_camera.setFar(g_camera.getFar());
}

void init() {
	initGLFW();
	initOpenGL();
	initCPUgeometry();
	initGPUprogram();
	initCamera();
}

void clear() {
	glDeleteProgram(g_program);

	glfwDestroyWindow(g_window);
	glfwTerminate();
}

// The main rendering call
void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

	const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
	const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();

	glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
	glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix)); // compute the projection matrix of the camera and pass it to the GPU program

	const glm::vec3 camPosition = g_camera.getPosition();
	glUniform3f(glGetUniformLocation(g_program, "camPos"), camPosition[0], camPosition[1], camPosition[2]);

	glActiveTexture(GL_TEXTURE0);
	for (int i = 0; i < nbPlanetsToRender; i++)
	{
		glBindTexture(GL_TEXTURE_2D, texIDs[i]);
		planets[i]->renderMesh();
	}
	//glBindTexture(GL_TEXTURE_2D, texIDs[2]);
	//earthSphere->renderMesh();

	glBindTexture(GL_TEXTURE_2D, g_moonTexID);
	moonSphere->renderMesh();

	// You will literally never see the difference if this is uncommented because the sun is a solid color
	//glBindTexture(GL_TEXTURE_2D, g_sunTexID);
	sunSphere->renderMesh();
}

// Update any accessible variable based on the current time
void update(const float currentTimeInSec) {
	if ((currentTimeInSec - lastUpdateTime) * fps > 1)
	{
		/*venusSphere->rotate(sunSphere.get(), Y_ROTATION_VECTOR, earthRotationSpeed / 1.6);
		venusSphere->rotate(venusSphere.get(), glm::vec3(10.0, 177.0, 0.0), 2*earthRotationSpeed/3);*/
		for (int i = 0; i < nbPlanetsToRender; i++)
		{
			planets[i]->rotateAround(sunSphere.get(), glm::vec3(orbitInclSin[i], orbitInclCos[i], 0.0), fpsSkip / (slowdownRatio * planetRotSun[i]));
			planets[i]->rotateAround(planets[i].get(), glm::vec3(axialTiltSin[i], axialTiltCos[i], 0.0), fpsSkip / (slowdownRatio * planetRotItself[i]));
		}
		//earthSphere->rotateAround(sunSphere.get(), Y_ROTATION_VECTOR, planetRotSun[2] / 1000);
		//earthSphere->rotateAround(earthSphere.get(), glm::vec3(10.0, axialTiltDegrees[2], 0.0), planetRotItself[2] / 1000);

		// The moon moves with the Earth. The Earth moves with the sun. By the transitive property, the moon moves with the sun.
		moonSphere->rotateAround(sunSphere.get(), Y_ROTATION_VECTOR, fpsSkip * planetRotSun[0] / slowdownRatio);
		moonSphere->rotateAround(planets[0].get(), Y_ROTATION_VECTOR, fpsSkip * planetRotSun[0] / slowdownRatio * 2);

		lastUpdateTime = currentTimeInSec;
	}
}

int main(int argc, char** argv) {
	init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)

	while (!glfwWindowShouldClose(g_window)) {
		update(static_cast<float>(glfwGetTime()));
		glfwPollEvents();
		render();
		glfwSwapBuffers(g_window);
		glfwPollEvents();
	}
	clear();
	return EXIT_SUCCESS;
}
