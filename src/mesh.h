#ifndef INCLUDE_MESH
#define INCLUDE_MESH

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
	void init(int resolution);

	/*
	* @brief Function called during the main rendering loop
	*/
	void renderMesh();

	/**
	* @brief Generates a sphere centered at (0,0,0) with sphereRadius 1.
	*
	* @param resolution The amount of points used to approximate a disk, and also amount of disks.
	*
	* @return The initialized Mesh
	*/
	inline static std::shared_ptr<Mesh> genSphere(const size_t resolution = 16)
	{
		// NOTE: This method is only called once to create a sphere, then rendered three times in different positions
		// With translations

		// Shared pointer used to auto free the pointer when no longer used
		std::shared_ptr<Mesh> sharedMeshPointer = std::make_shared<Mesh>() ;
		sharedMeshPointer->init(resolution);
		return sharedMeshPointer;
	}

private:
	std::vector<float> m_vertexPositions;
	std::vector<float> m_vertexColors;
	std::vector<float> m_vertexNormals;
	std::vector<unsigned int> m_triangleIndices;
	GLuint m_vao = 0;
	GLuint m_posVbo = 0;
	GLuint m_normVbo = 0;
	GLuint m_colVbo = 0;
	GLuint m_normalVbo = 0;
	GLuint m_ibo = 0;

	// Amount of points used to approximate a disk, and also amount of disks.
	int size = 16;
	int nbPoints;

	/**
	* @brief Defines a point's position given its x,y,z coordinates.
	*
	* @param position The point to assign the coordinates to.
	* @param x The point's x-coordinate.
	* @param y The point's y-coordinate.
	* @param z The point's z-coordinate.
	*/
	void definePointPosition(int position, float x, float y, float z);

	void defineColor(int position, float x, float y, float z);

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

	/*
	* @brief Defines how the mesh will be displayed on screen.
	*/
	void defineRenderMethod();
};
#endif