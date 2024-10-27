#ifndef INCLUDE_MESH
#define INCLUDE_MESH

#include "meshUtility.h"

#include <dep/glm/glm.hpp>
#include <dep/glm/gtc/matrix_transform.hpp>
#include <dep/glm/gtc/type_ptr.hpp>

#include <iostream>
#include <glad/gl.h>
#include <memory>
#include <vector>
#include <cassert>
#include <corecrt_math_defines.h>

class Mesh
{
public:
	/*
	* @brief Declares the different vectors that store the Mesh information, and then creates the mesh.
	* 
	* @param resolution The amount of points used to approximate a disk, and also amount of disks.
	*/
	void init(const size_t resolution);

	/*
	* @brief Sets up the sun-specific parameters.
	*/
	void setupSun();

	/*
	* @brief Sets up the planet-specific parameters
	* 
	* @param angleOfRotationAxis The angle at which the planet rotates around.
	*/
	void setupPlanet(double angleOfRotationAxis, double orbitProgress);

	/*
	* @brief Function called during the main rendering loop
	*/
	void renderMesh();

	/*
	* @brief Rotate around a body.
	* 
	* Note that this rotation is both around the orbitingBody and the body itself.
	* 
	* @param *orbitingBody The pointer to the body the current body is orbiting around
	* @param axisVector The axis to spin around
	* @param rotationSpeed The speed of rotation around the body
	*/
	void rotateAround(Mesh* orbitingBody, glm::vec3 axisVector, float rotationSpeed);

	/*
	* @brief Rotate around a body.
	*
	* Note that this rotation is both around the orbitingBody and the body itself.
	*
	* @param obCenter The coordinates of the center of the body the current body is orbiting around.
	* @param axisVector The axis to spin around
	* @param rotationSpeed The speed of rotation around the body
	*/
	void rotateAround(glm::vec3 obCenter, glm::vec3 axisVector, float rotationSpeed);

	/*
	* @brief Move a body linearly. rotate() also 
	* 
	* @param matxMove The transformation matrix describing how to move the body
	*/
	void move(glm::mat4 matxMove);

	/*
	* @brief Defines how the mesh will be displayed on screen.
	*/
	void defineRenderMethod();

	/**
	* @brief Generates a sphere centered at (0,0,0) with sphereRadius 1 and sets the position of the sun.
	*
	* @param x The x-coordinate of the sun.
	* @param y The y-coordinate of the sun.
	* @param z The z-coordinate of the sun.
	* @param resolution The amount of points used to approximate a disk, and also amount of disks. Defaults to 16.
	*
	* @return The initialized Mesh
	*/
	inline static std::shared_ptr<Mesh> genSphere(float x, float y, float z, const size_t resolution = 16)
	{
		// This method is only called once to create a sphere, then rendered three times in different positions
		// With translations

		// Shared pointer used to auto free the pointer when no longer used
		std::shared_ptr<Mesh> sharedMeshPointer = std::make_shared<Mesh>();
		sharedMeshPointer->init(resolution);
		sharedMeshPointer->setSunCenter(x, y, z);
		return sharedMeshPointer;
	}

	/*
	* @brief Get the center of the body.
	* 
	* @return The coordinate of the center of the body.
	*/
	glm::vec3 getSelfCenter() const;

private:
	// The position of the vertices, not the triangles
	std::vector<glm::vec3> m_vertexPositions;

	// The color at the vertices, not the global color of the triangle
	std::vector<glm::vec3> m_vertexNormals;
	std::vector<glm::vec3> m_vertexLight;
	std::vector<glm::vec3> m_vertexAmbience;
	std::vector<glm::vec2> m_vertexTexCoords;

	std::vector<unsigned int> m_triangleIndices;
	GLuint m_vao = 0;

	GLuint m_posVbo = 0;
	GLuint m_normVbo = 0;
	GLuint m_lightVbo = 0;
	GLuint m_ambiVbo = 0;
	GLuint m_texVbo = 0;

	GLuint m_ibo = 0;

	// Coordinates of the sun
	glm::vec3 sun_center, self_center;

	// Amount of points used to approximate a disk, and also amount of disks.
	int size = 16;
	int nbPoints=0;

	float rotationalAxis = 0.0f;

	/**
	* @brief Defines a point's position given its x,y,z coordinates.
	*
	* @param position The point to assign the coordinates to.
	* @param x The point's x-coordinate.
	* @param y The point's y-coordinate.
	* @param z The point's z-coordinate.
	*/
	void definePointPosition(int position, float x, float y, float z);

	/**
	* @brief Defines a triangle's vertexes.
	*
	* @param position The triangle to assign the vertexes to.
	* @param pt1 The triangle's first point.
	* @param pt2 The triangle's second point.
	* @param pt3 The triangle's third point.
	*/
	void defineTrianglePoints(int position, int pt1, int pt2, int pt3);

	void defineTextureCoord(int position, float x, float y);

	/**
	* @brief Sets up the position and the colors of the points in the mesh
	*/
	void definePositionsAndColor();

	/**
	* @brief Associates the texture to the vertex
	*/
	void defineTextureCoords();

	/*
	* @brief Defines the triangles by defining their points.
	*/
	void defineIndices();

	
	/**
	* @brief Send a vector list to the vertexShader.glsl for processing.
	* 
	* @param m_vertexInfo The vector list
	* @param *vbo The pointer to the VBO value
	* @param location The channel that vertexShader will recieve this information on.
	*/
	template <typename T>
	void sendVertexShader(std::vector<T> m_vextexInfo, GLuint *vbo, int location);

	/*
	* @brief Update the data being sent to vertexShader.glsl
	* 
	* @param m_vertexInfo The data replacing the old data being sent to the vertex shader.
	* @param *vbo The pointer associated with the location transmitting the old data.
	*/
	void updateRendering(std::vector<glm::vec3> m_vextexInfo, GLuint *vbo);

	/*
	* @brief Set the coordinates of the location of the sun's center.
	* This function is private because it should only be accessed by genSphere()
	*/
	inline void setSunCenter(float x, float y, float z)
	{
		sun_center = glm::vec3(x, y, z);
	}

	/*
	* @brief A general purpose function for applying (or creating) a transformation matrix.
	* 
	* This function serves to either apply a translation matrix, apply a rotation matrix around an axis.
	* 
	* @param matxTrans The transformation matrix describing how to move the body
	*/
	void transform(glm::mat4 matxTrans);
};
#endif