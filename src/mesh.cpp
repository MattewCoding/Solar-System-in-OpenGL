#include "mesh.h"


void Mesh::definePointPosition(int position, float x, float y, float z)
{
	assert(position % 3 == 0);
	int realPos = position;// *3;

	m_vertexPositions[realPos] = x;
	m_vertexNormals[realPos] = x;

	m_vertexPositions[realPos + 1] = y;
	m_vertexNormals[realPos + 1] = y;

	m_vertexPositions[realPos + 2] = z;
	m_vertexNormals[realPos + 2] = z;
}

void Mesh::defineColor(int position, float red, float green, float blue)
{
	assert(position % 3 == 0);
	m_vertexColors[position] = red;
	m_vertexColors[position + 1] = green;
	m_vertexColors[position + 2] = blue;
}

void Mesh::defineTrianglePoints(int position, int pt1, int pt2, int pt3)
{
	assert(position % 3 == 0);
	int realPos = position;// *3;
	m_triangleIndices[realPos] = pt1;
	m_triangleIndices[realPos + 1] = pt2;
	m_triangleIndices[realPos + 2] = pt3;
}

void Mesh::definePositionsAndColor()
{
	int i = 0;

	//std::cout << "Nb of points: " << nbPoints << std::endl;

	int thetaIndex = 0;
	int phiIndex = 0;

	// North point
	definePointPosition(i, 0, 0, 1);
	defineColor(i, 0, 0, 1);
	i += 3;
	//std::cout << "First point: " << i / 3 << "; " << m_vertexPositions[i - 3] << ", " << m_vertexPositions[i - 2] << ", " << m_vertexPositions[i - 1] << std::endl;

	std::vector<float> sinTheta(size), cosTheta(size);
	std::vector<float> sinPhi(size), cosPhi(size);

	for (int index = 0; index < size; ++index) {
		float theta = M_PI * index / (size - 1); // 0-indexing => size differs
		sinTheta[index] = sin(theta);
		cosTheta[index] = cos(theta);

		float phi = 2. * M_PI * index / size;
		sinPhi[index] = sin(phi);
		cosPhi[index] = cos(phi);
	}

	for (thetaIndex = 1; thetaIndex < size - 1; thetaIndex++)
	{
		for (phiIndex = 0; phiIndex < size; phiIndex++, i += 3)
		{
			float x = sinTheta[thetaIndex] * cosPhi[phiIndex];
			float y = sinTheta[thetaIndex] * sinPhi[phiIndex];
			float z = cosTheta[thetaIndex];
			definePointPosition(i, x, y, z);
			defineColor(i, x, y, z);

			//std::cout << "Curr point: " << i / 3 << "; " << m_vertexPositions[i - 3] << ", " << m_vertexPositions[i - 2] << ", " << m_vertexPositions[i - 1] << std::endl;
		}
	}

	// South point
	definePointPosition(i, 0, 0, -1);
	defineColor(i, 0, 0, 1);
	//std::cout << "Last point: " << i / 3 << "; " << m_vertexPositions[i - 3] << ", " << m_vertexPositions[i - 2] << ", " << m_vertexPositions[i - 1] << std::endl;
}

void Mesh::defineIndices()
{
	// Top part
	int currFaceVertex = 0; // currFace = (int)(currFaceVertex/3);
	int lastTopFV = (size - 1) * 3;
	while (currFaceVertex < lastTopFV)
	{
		defineTrianglePoints(currFaceVertex, 0, currFaceVertex / 3 + 1, currFaceVertex / 3 + 2);
		currFaceVertex += 3;
	}
	defineTrianglePoints(currFaceVertex, 0, currFaceVertex / 3 + 1, 1);
	currFaceVertex += 3;

	for (int currRow = 0; currRow < size - 3; currRow++)
	{
		for (int currCol = 1; currCol < size + 1; currCol++)
		{
			// Example: 1-6-2 and 2-6-7 for 5 points a
			int thirdPt = (currCol != size) ? currRow * size + currCol + 1 : currRow * size + 1;
			defineTrianglePoints(currFaceVertex, currRow * size + currCol, (currRow + 1) * size + currCol, thirdPt);
			currFaceVertex += 3;

			int firstPt = (currCol != size) ? currRow * size + currCol + 1 : currRow * size + 1;
			thirdPt = (currCol != size) ? (currRow + 1) * size + currCol + 1 : currRow * size + currCol + 1;
			defineTrianglePoints(currFaceVertex, firstPt, (currRow + 1) * size + currCol, thirdPt);
			currFaceVertex += 3;
		}
	}

	// Bottom part
	lastTopFV = (3 * 2 * (nbPoints - 2) - currFaceVertex) / 3;
	//std::cout << currFaceVertex << "; " << 3 * 2 * (nbPoints - 2) << std::endl;

	// Note: This is DELIBERATELY backwards 'cause it's facing AWAY from the screen
	for (int i = 0; i < lastTopFV; i++)
	{
		// Bottom point
		defineTrianglePoints(currFaceVertex, nbPoints - 1, nbPoints - size + i, nbPoints - 1 - size + i);
		currFaceVertex += 3;
	}
	//defineTrianglePoints(currFaceVertex, nbPoints - 1, nbPoints - 1 - size, nbPoints - 2 - size + lastTopFV);
}

void Mesh::sendVertexShader(std::vector<float> m_vextexInfo, GLuint vbo, int location)
{
	size_t bufferSize = sizeof(float) * m_vextexInfo.size(); // Gather the size of the buffer from the CPU-side vector

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, m_vextexInfo.data(), GL_DYNAMIC_READ);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(location);
}

void Mesh::defineRenderMethod()
{
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	sendVertexShader(m_vertexPositions, m_posVbo, 0);
	sendVertexShader(m_vertexNormals, m_normVbo, 1);
	sendVertexShader(m_vertexColors, m_colVbo, 2);

	size_t indexBufferSize = sizeof(unsigned int) * m_triangleIndices.size();

	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, m_triangleIndices.data(), GL_DYNAMIC_READ);
}

void Mesh::init(int resolution)
{
	size = resolution;

	nbPoints = size * (size - 2) + 2;
	m_vertexPositions = std::vector<float>(nbPoints * 3);
	m_vertexColors = std::vector<float>(nbPoints * 3);
	m_vertexNormals = std::vector<float>(nbPoints * 3);

	// nb_of_pts_in_triangle * each_rect_has_two_triangles * pts_excl_poles
	m_triangleIndices = std::vector<unsigned int>(3 * 2 * (nbPoints - 2));

	definePositionsAndColor();
	defineIndices();
	defineRenderMethod();

	/*for (int i = 0; i < 3 * 2 * (nbPoints - 2); i++)
	{
		if (i % 3 == 0) std::cout << std::endl;
		std::cout << m_triangleIndices[i] << " ";
	}*/
}

void Mesh::renderMesh()
{
	glBindVertexArray(m_vao);     // activate the VAO storing geometry data
	glDrawElements(GL_TRIANGLES, m_triangleIndices.size(), GL_UNSIGNED_INT, 0);
}

void Mesh::setUniformColor(float red, float green, float blue)
{
	for (int i = 0; i < nbPoints * 3; i += 3)
	{
		defineColor(i, red, green, blue);
	}
	defineRenderMethod(); // have to update the render color
}

void Mesh::transform(glm::mat4 matxTrans) {
	for (int i = 0; i < nbPoints * 3; i += 3) {
		glm::vec4 pointCoord{ m_vertexPositions[i], m_vertexPositions[i + 1], m_vertexPositions[i + 2], 1};
		pointCoord = matxTrans * pointCoord;
		m_vertexPositions[i] = pointCoord[0];
		m_vertexPositions[i + 1] = pointCoord[1];
		m_vertexPositions[i + 2] = pointCoord[2];
	}
	defineRenderMethod(); // have to update positions
}
