#include "mesh.h"


void Mesh::definePointPosition(int position, float x, float y, float z)
{
	int realPos = position;

	m_vertexPositions[realPos] = glm::vec3(x,y,z);
	m_vertexNormals[realPos] = glm::vec3(x,y,z);
}

void Mesh::defineColor(int position, float red, float green, float blue)
{
	m_vertexColors[position] = glm::vec3(red, green, blue);
}

void Mesh::defineTrianglePoints(int position, int pt1, int pt2, int pt3)
{
	int realPos = position;
	m_triangleIndices[realPos] = pt1;
	m_triangleIndices[realPos + 1] = pt2;
	m_triangleIndices[realPos + 2] = pt3;
}

void Mesh::definePositionsAndColor()
{
	self_center = glm::vec3(0., 0., 0.);

	int i = 0;

	//std::cout << "Nb of points: " << nbPoints << std::endl;

	int thetaIndex = 0;
	int phiIndex = 0;

	// North point
	definePointPosition(i, 0, 0, 1);
	defineColor(i, 0, 0, 1);
	i++;
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
		for (phiIndex = 0; phiIndex < size; phiIndex++, i++)
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

void Mesh::sendVertexShader(std::vector<glm::vec3> m_vextexInfo, GLuint *vbo, int location)
{
	size_t bufferSize = sizeof(glm::vec3) * m_vextexInfo.size(); // Gather the size of the buffer from the CPU-side vector

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, m_vextexInfo.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(location);
}

void Mesh::defineRenderMethod()
{
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	sendVertexShader(m_vertexPositions, &m_posVbo, 0);
	sendVertexShader(m_vertexNormals, &m_normVbo, 1);
	sendVertexShader(m_vertexColors, &m_colVbo, 2);
	sendVertexShader(m_vertexLight, &m_lightVbo, 3);
	sendVertexShader(m_vertexAmbience, &m_ambiVbo, 4);

	size_t indexBufferSize = sizeof(unsigned int) * m_triangleIndices.size();

	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, m_triangleIndices.data(), GL_DYNAMIC_DRAW);

	glBindVertexArray(0); // Unbinding
}

void Mesh::init(const size_t resolution)
{
	size = resolution;

	nbPoints = size * (size - 2) + 2;
	m_vertexPositions = std::vector<glm::vec3>(nbPoints);
	m_vertexColors = std::vector<glm::vec3>(nbPoints);
	m_vertexNormals = std::vector<glm::vec3>(nbPoints);
	m_vertexLight = std::vector<glm::vec3>(nbPoints);
	m_vertexAmbience = std::vector<glm::vec3>(nbPoints);

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
	glBindVertexArray(0); // Unbinding
}

void Mesh::setUniformColor(float red, float green, float blue)
{
	for (int i = 0; i < nbPoints; i++)
	{
		defineColor(i, red, green, blue);
	}
	defineRenderMethod(); // have to update the render color
}

void Mesh::updateRendering(std::vector<glm::vec3> m_vextexInfo, GLuint *vbo)
{
	size_t bufferSize = sizeof(glm::vec3) * m_vextexInfo.size(); // Gather the size of the buffer from the CPU-side vector

	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * m_vextexInfo.size(), m_vextexInfo.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::rotateAroundBody(Mesh* orbitingBody, float rotationSpeed)
{
	transform(glm::mat4(1.), this, -rotationSpeed);
	transform(glm::mat4(1.), orbitingBody, rotationSpeed);
}

void Mesh::rotate(Mesh* orbitingBody, float rotationSpeed)
{
	transform(glm::mat4(1.), orbitingBody, rotationSpeed);
}

void Mesh::move(glm::mat4 matxMove)
{
	transform(matxMove, NULL, 0);
}

void Mesh::transform(glm::mat4 matxTrans, Mesh* orbitingBody, float rotationSpeed)
{
	if (orbitingBody != NULL)
	{
		const float cosRot = (float)cos(M_PI / rotationSpeed);
		const float sinRot = (float)sin(M_PI / rotationSpeed);

		// Ensure rotating around body center, not world center
		matxTrans[0] = glm::vec4(cosRot, sinRot, 0, 0);
		matxTrans[1] = glm::vec4(-sinRot, cosRot, 0, 0);
		//transform(glm::mat4(1.), NULL, -rotationSpeed);
		glm::vec3 orbitingBodyCenter = orbitingBody->getSelfCenter();
		matxTrans[3] = glm::vec4(orbitingBodyCenter.x * (1 - cosRot) + orbitingBodyCenter.y * sinRot, orbitingBodyCenter.y * (1 - cosRot) - orbitingBodyCenter.x * sinRot, 0, 1);
		
	}

	self_center = glm::vec3(matxTrans * glm::vec4{ self_center, 1 });

	for (int i = 0; i < nbPoints; i++) {
		glm::vec4 pointCoord{ m_vertexPositions[i], 1};
		pointCoord = matxTrans * pointCoord;
		m_vertexPositions[i] = glm::vec3(pointCoord);
		m_vertexNormals[i] = glm::vec3(pointCoord) - self_center;

		m_vertexLight[i] = glm::vec3(
			sun_center.x - pointCoord.x,
			sun_center.y - pointCoord.y,
			sun_center.z - pointCoord.z
		);
	}

	//std::cout << m_vertexPositions[0].x << ", " << m_vertexPositions[0].y << ", " << m_vertexPositions[0].z << "; ";
	//std::cout << m_vertexNormals[0].x << ", " << m_vertexNormals[0].y << ", " << m_vertexNormals[0].z << std::endl;

	glBindVertexArray(m_vao);
	updateRendering(m_vertexPositions, &m_posVbo);
	updateRendering(m_vertexNormals, &m_normVbo);
	updateRendering(m_vertexLight, &m_lightVbo);
	glBindVertexArray(0); // Unbinding
}

void Mesh::setupSun()
{
	for (int i = 0; i < nbPoints; i++) {
		m_vertexAmbience[i] = glm::vec3(1., 1., 0.);
	}

	glBindVertexArray(m_vao);
	updateRendering(m_vertexAmbience, &m_ambiVbo);
	glBindVertexArray(0); // Unbinding
}

glm::vec3 Mesh::getSelfCenter()
{
	return self_center;
}