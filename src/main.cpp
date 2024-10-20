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

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <dep/glm/glm.hpp>
#include <dep/glm/ext.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>


// constants
const static float kSizeSun = 1;
const static float kSizeEarth = 0.5;
const static float kSizeMoon = 0.25;
const static float kRadOrbitEarth = 10;
const static float kRadOrbitMoon = 2;

// Window parameters
GLFWwindow* g_window = nullptr;

// GPU objects
GLuint g_program = 0; // A GPU program contains at least a vertex shader and a fragment shader

// Basic camera model
Camera g_camera;

// Toy mesh for a sphere
std::shared_ptr<Mesh> sphereMesh;

GLuint loadTextureFromFileToGPU(const std::string& filename) {
	int width, height, numComponents;
	// Loading the image in CPU memory using stb_image
	unsigned char* data = stbi_load(
		filename.c_str(),
		&width, &height,
		&numComponents, // 1 for a 8 bit grey-scale image, 3 for 24bits RGB image, 4 for 32bits RGBA image
		0);

	GLuint texID;
	// TODO: create a texture and upload the image data in GPU memory using
	// glGenTextures, glBindTexture, glTexParameteri, and glTexImage2D

	// Free useless CPU memory
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture

	return texID;
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow* window, int width, int height) {
	g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
	glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS && key == GLFW_KEY_W) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_F) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else if (action == GLFW_PRESS && (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)) {
		glfwSetWindowShouldClose(window, true); // Closes the application if the escape key is pressed
	}
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
		1024, 768,
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
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f); // specify the background color, used any time the framebuffer is cleared
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

	std::string backoutPath = "../../../";
	loadShader(g_program, GL_VERTEX_SHADER, backoutPath + "vertexShader.glsl"); //TODO: FIND WHERE THE OUT OPTION IS
	loadShader(g_program, GL_FRAGMENT_SHADER, backoutPath + "fragmentShader.glsl");
	glLinkProgram(g_program); // The main GPU program is ready to be handle streams of polygons

	glUseProgram(g_program);
	// TODO: set shader variables, textures, etc.
}

// Define your mesh(es) in the CPU memory
void initCPUgeometry() {
	// Reminder: this is here and not earlier because the program needs to init the shaders 'n stuff.
	sphereMesh = Mesh::genSphere();
}

void initCamera() {
	int width, height;
	glfwGetWindowSize(g_window, &width, &height);
	g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

	g_camera.setPosition(glm::vec3(0.0, 0.0, 3.0));
	g_camera.setNear(0.1);
	g_camera.setFar(80.1);
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

	sphereMesh->renderMesh();
}

// Update any accessible variable based on the current time
void update(const float currentTimeInSec) {
	// std::cout << currentTimeInSec << std::endl;

}

int main(int argc, char** argv) {
	init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
	while (!glfwWindowShouldClose(g_window)) {
		update(static_cast<float>(glfwGetTime()));
		render();
		glfwSwapBuffers(g_window);
		glfwPollEvents();
	}
	clear();
	return EXIT_SUCCESS;
}
