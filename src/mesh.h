#ifndef INCLUDE_MESH
#define INCLUDE_MESH

#include <iostream>
#include <glad/gl.h>
#include <memory>
#include <vector>
#include <cassert>
#include <corecrt_math_defines.h>
#include <dep/glm/glm.hpp>

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
	* @brief Function called during the main rendering loop
	*/
	void renderMesh();

	/*
	* @brief Set the sphere to be one same color.
	* 
	* @param red The amount of red. Ranges from 0 to 1 inclusive.
	* @param green The amount of green. Ranges from 0 to 1 inclusive.
	* @param blue The amount of blue. Ranges from 0 to 1 inclusive.
	*/
	void setUniformColor(float red, float green, float blue);

	/*
	* @brief Rotate around a body.
	*
	* Note that this rotation is only around the orbitingBody and not the body itself.
	* For rotation around the orbitingBody and itself, try rotate()
	*
	* @param orbitingBody The body the current body is orbiting around
	* @param rotationSpeed The speed of rotation around the body
	*/
	void rotateAroundBody(Mesh* orbitingBody, float rotationSpeed);

	/*
	* @brief Rotate around a body.
	* 
	* Note that this rotation is both around the orbitingBody and the body itself.
	* For rotation purely around the orbitingBody, try rotateAroundBody()
	* 
	* @param orbitingBody The body the current body is orbiting around
	* @param rotationSpeed The speed of rotation around the body
	*/
	void rotate(Mesh* orbitingBody, float rotationSpeed);

	/*
	* @brief Move a body linearlly
	* 
	* @param matxMove The transformation matrix describing how to move the body
	*/
	void move(glm::mat4 matxMove);

	/*
	* @brief Function that sets up the sun-specific parameters.
	*/
	void setupSun();

	glm::vec3 getSelfCenter();

	/**
	* @brief Generates a sphere centered at (0,0,0) with sphereRadius 1.
	*
	* @param resolution The amount of points used to approximate a disk, and also amount of disks.
	*
	* @return The initialized Mesh
	*/
	inline static std::shared_ptr<Mesh> genSphere(float x, float y, float z, const size_t resolution = 4)
	{
		// NOTE: This method is only called once to create a sphere, then rendered three times in different positions
		// With translations

		// Shared pointer used to auto free the pointer when no longer used
		std::shared_ptr<Mesh> sharedMeshPointer = std::make_shared<Mesh>() ;
		sharedMeshPointer->init(resolution);
		sharedMeshPointer->setSunCenter(x, y, z);
		return sharedMeshPointer;
	}

private:
	// The position of the vertices, not the triangles
	std::vector<glm::vec3> m_vertexPositions;

	// The color at the vertices, not the global color of the triangle
	std::vector<glm::vec3> m_vertexColors;
	std::vector<glm::vec3> m_vertexNormals;
	std::vector<glm::vec3> m_vertexLight;
	std::vector<glm::vec3> m_vertexAmbience;

	std::vector<unsigned int> m_triangleIndices;
	GLuint m_vao = 0;

	GLuint m_posVbo = 0;
	GLuint m_normVbo = 0;
	GLuint m_colVbo = 0;
	GLuint m_lightVbo = 0;
	GLuint m_ambiVbo = 0;

	GLuint m_ibo = 0;

	// Coordinates of the sun
	glm::vec3 sun_center, self_center;

	// Amount of points used to approximate a disk, and also amount of disks.
	int size = 16;
	int nbPoints=0;

	/**
	* @brief Defines a point's position given its x,y,z coordinates.
	*
	* @param position The point to assign the coordinates to.
	* @param x The point's x-coordinate.
	* @param y The point's y-coordinate.
	* @param z The point's z-coordinate.
	*/
	void definePointPosition(int position, float x, float y, float z);

	/*
	* @brief Defines a point's color.
	*
	* @param red The amount of red. Ranges from 0 to 1 inclusive.
	* @param green The amount of green. Ranges from 0 to 1 inclusive.
	* @param blue The amount of blue. Ranges from 0 to 1 inclusive.
	*/
	void defineColor(int position, float red, float green, float blue);

	/**
	* @brief Defines a triangle's vertexes.
	*
	* @param position The triangle to assign the vertexes to.
	* @param pt1 The triangle's first point.
	* @param pt2 The triangle's second point.
	* @param pt3 The triangle's third point.
	*/
	void defineTrianglePoints(int position, int pt1, int pt2, int pt3);

	/**
	* @brief Sets up the position and the colors of the points in the mesh
	*/
	void definePositionsAndColor();

	/*
	* @brief Defines the triangles by defining their points.
	*/
	void defineIndices();

	/**
	* @brief Send a vector list to the vertexShader.glsl for processing.
	* 
	* @param m_vertexInfo The vector list
	* @param vbo The pointer to the VBO value
	* @param location The channel that vertexShader will recieve this information on.
	*/
	void sendVertexShader(std::vector<glm::vec3> m_vextexInfo, GLuint *vbo, int location);

	/*
	* @brief Defines/updates how the mesh will be displayed on screen.
	*/
	void defineRenderMethod();
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
	* This function serves to either apply a translation matrix, or create and apply a rotation matrix around a point.
	* Note that, due to implementation, a pointer to NULL assumes a translation matrix is being applied.
	* 
	* @param matxMove The transformation matrix describing how to move the body
	* @param orbitingBody The body the current body is orbiting around
	* @param rotationSpeed The speed of rotation around the body
	*/
	void transform(glm::mat4 initMatx, Mesh* orbitingBody, float rotationSpeed);
};
#endif