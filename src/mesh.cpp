#include "mesh.h"


void Mesh::definePointPosition(int position, float x, float y, float z)
{
	int realPos = position;

	m_vertexPositions[realPos] = glm::vec3(x,y,z);
	m_vertexNormals[realPos] = glm::vec3(x,y,z);
}

void Mesh::defineTrianglePoints(int position, int pt1, int pt2, int pt3)
{
	int realPos = position;
	m_triangleIndices[realPos] = pt1;
	m_triangleIndices[realPos + 1] = pt2;
	m_triangleIndices[realPos + 2] = pt3;
}

void Mesh::defineTextureCoord(int position, float x, float y)
{
	m_vertexTexCoords.push_back(glm::vec2(x, y));
}

void Mesh::definePositionsAndColor()
{
	self_center = glm::vec3(0., 0., 0.);

	int i = 0;
	int thetaIndex = 0;
	int phiIndex = 0;

	// North point
	definePointPosition(i, 0, 0, 1);
	i++;

	const int sizePlusOne = size + 1;
	std::vector<float> sinTheta(sizePlusOne), cosTheta(sizePlusOne);
	std::vector<float> sinPhi(sizePlusOne), cosPhi(sizePlusOne);

	for (int index = 0; index < size; ++index) {
		float theta = M_PI * index / (size - 1); // 0-indexing => size differs
		sinTheta[index] = sin(theta);
		cosTheta[index] = cos(theta);

		float phi = 2. * M_PI * index / size;
		sinPhi[index] = sin(phi);
		cosPhi[index] = cos(phi);
	}

	sinTheta[size] = sinTheta[0];
	cosTheta[size] = cosTheta[0];
	sinPhi[size] = sinPhi[0];
	cosPhi[size] = cosPhi[0];

	for (thetaIndex = 1; thetaIndex < size - 1; thetaIndex++)
	{
		for (phiIndex = 0; phiIndex < size + 1; phiIndex++, i++)
		{
			float x = sinTheta[thetaIndex] * cosPhi[phiIndex];
			float y = sinTheta[thetaIndex] * sinPhi[phiIndex];
			float z = cosTheta[thetaIndex];
			definePointPosition(i, x, y, z);
		}
	}

	// South point
	definePointPosition(i, 0, 0, -1);
}

void Mesh::defineTextureCoords()
{
	int index = 0;

	defineTextureCoord(index, 0.5, 0.0);
	index++;
	for (int x = 1; x < size - 1; x++)
	{
		for (int y = 0; y < size+1; y++)
		{
			float u = static_cast<float>(y) / (size);
			float v = static_cast<float>(x) / (size - 1);
			defineTextureCoord(index, u, v);
			index++;
		}
	}
	defineTextureCoord(index, 0.5, 1.0);
}

void Mesh::defineIndices()
{
	// Top part
	int currFaceVertex = 0;
	int lastTopFV = size * 3;
	while (currFaceVertex < lastTopFV)
	{
		defineTrianglePoints(currFaceVertex, 0, currFaceVertex / 3 + 1, currFaceVertex / 3 + 2);
		currFaceVertex += 3;
	}

	const int sizePlusOne = size + 1;
	for (int currRow = 0; currRow < size - 3; currRow++)
	{
		for (int currCol = 1; currCol < size + 1; currCol++)
		{
			// Example: 1-6-2 and 2-6-7 for 5 points a
			defineTrianglePoints(currFaceVertex, currRow * sizePlusOne + currCol, (currRow + 1) * sizePlusOne + currCol, currRow * sizePlusOne + currCol + 1);
			currFaceVertex += 3;

			defineTrianglePoints(currFaceVertex, currRow * sizePlusOne + currCol + 1, (currRow + 1) * sizePlusOne + currCol, (currRow + 1) * sizePlusOne + currCol + 1);
			currFaceVertex += 3;
		}
	}

	// This isn't working
	// lastTopFV is not correct

	// Bottom part
	lastTopFV = (3 * 2 * size * (size - 2) - currFaceVertex) / 3;

	// Note: This is DELIBERATELY backwards 'cause it's facing AWAY from the screen
	for (int i = 0; i < lastTopFV - 1; i++)
	{
		// Bottom point
		defineTrianglePoints(currFaceVertex, nbPoints - 1, nbPoints - sizePlusOne + i, nbPoints - 1 - sizePlusOne + i);
		currFaceVertex += 3;
	}
	defineTrianglePoints(currFaceVertex, nbPoints - 1, nbPoints - 1 - sizePlusOne, nbPoints - 2 - sizePlusOne + lastTopFV);

}

template <typename T>
void Mesh::sendVertexShader(std::vector<T> m_vertexInfo, GLuint* vbo, int location)
{
	size_t bufferSize = sizeof(T) * m_vertexInfo.size(); // Gather the size of the buffer from the CPU-side vector

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, m_vertexInfo.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, sizeof(T), 0);
	glEnableVertexAttribArray(location);
}

void Mesh::defineRenderMethod()
{
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	sendVertexShader(m_vertexPositions, &m_posVbo, 0);
	sendVertexShader(m_vertexNormals, &m_normVbo, 1);
	sendVertexShader(m_vertexLight, &m_lightVbo, 2);
	sendVertexShader(m_vertexAmbience, &m_ambiVbo, 3);
	sendVertexShader(m_vertexTexCoords, &m_texVbo, 4);

	size_t indexBufferSize = sizeof(unsigned int) * m_triangleIndices.size();

	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, m_triangleIndices.data(), GL_DYNAMIC_DRAW);

	glBindVertexArray(0); // Unbinding
}

void Mesh::init(const size_t resolution)
{
	size = resolution;

	nbPoints = (size + 1) * (size - 2) + 2;
	m_vertexPositions = std::vector<glm::vec3>(nbPoints);
	m_vertexNormals = std::vector<glm::vec3>(nbPoints);
	m_vertexLight = std::vector<glm::vec3>(nbPoints);
	m_vertexAmbience = std::vector<glm::vec3>(nbPoints);

	// size = 3 * 2 * ( nbPoints - n-2 overlapping points - 2 pole points )
	m_triangleIndices = std::vector<unsigned int>(3 * 2 * size * (size - 2));

	definePositionsAndColor();
	defineTextureCoords();
	defineIndices();
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

void Mesh::setupPlanet(double angleOfRotationAxis, double orbitProgress, double orbitInclination)
{
	rotationalAxis = angleOfRotationAxis;

	rotateAround(this, X_ROTATION_VECTOR, -M_PI/2);
	rotateAround(this, Z_ROTATION_VECTOR, rotationalAxis);
	rotateAround(sun_center, Z_ROTATION_VECTOR, -orbitInclination);
	rotateAround(sun_center, glm::vec3(sin(orbitInclination), cos(orbitInclination), 0.0), orbitProgress);
}

void Mesh::renderMesh()
{
	glBindVertexArray(m_vao);     // activate the VAO storing geometry data
	glDrawElements(GL_TRIANGLES, m_triangleIndices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0); // Unbinding
}

void Mesh::updateRendering(std::vector<glm::vec3> m_vextexInfo, GLuint *vbo)
{
	size_t bufferSize = sizeof(glm::vec3) * m_vextexInfo.size(); // Gather the size of the buffer from the CPU-side vector

	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * m_vextexInfo.size(), m_vextexInfo.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::rotateAround(glm::vec3 obCenter, glm::vec3 axisVector, float rotationSpeed)
{
	glm::mat4 matxTrans{ glm::mat4(1.0) };
	const glm::vec3 selfCenter = getSelfCenter();

	if (obCenter != getSelfCenter() && rotationalAxis != 0.0f) // only if need to unrotate axis
	{
		glm::mat4 unrotateMatx =
			// Then moving into orbiting space and doing the orbital rotation
			MeshUtility::translate(obCenter) *
			MeshUtility::rotateAroundAxis(axisVector, rotationSpeed) *
			MeshUtility::translate(-obCenter) *

			// First, removing axial tilt by centering at world space
			MeshUtility::translate(selfCenter - obCenter) *
			MeshUtility::rotateAroundAxis(Z_ROTATION_VECTOR, -rotationalAxis) *
			MeshUtility::translate(obCenter - selfCenter);

		glm::vec3 newSelfCenter = glm::vec3{ unrotateMatx * glm::vec4{selfCenter, 1.0f} };

		matxTrans =
			MeshUtility::translate(newSelfCenter - obCenter) *
			MeshUtility::rotateAroundAxis(Z_ROTATION_VECTOR, rotationalAxis) *
			MeshUtility::translate(obCenter - newSelfCenter) *
			unrotateMatx;
	}
	else
	{
		matxTrans = MeshUtility::translate(obCenter) *
			MeshUtility::rotateAroundAxis(axisVector, rotationSpeed) *
			MeshUtility::translate(-obCenter);
	}

	transform(matxTrans);
}

void Mesh::rotateAround(Mesh * orbitingBody, glm::vec3 axisVector, float rotationSpeed)
{
	rotateAround(orbitingBody->getSelfCenter(), axisVector, rotationSpeed);
}

void Mesh::move(glm::mat4 matxMove)
{
	transform(matxMove);
}

void Mesh::transform(glm::mat4 matxTrans)
{
	self_center = glm::vec3(matxTrans * glm::vec4{ self_center, 1 });

	for (int i = 0; i < nbPoints; i++) {
		glm::vec3 pointCoord = glm::vec3(matxTrans * glm::vec4{ m_vertexPositions[i], 1 });
		m_vertexPositions[i] = pointCoord;
		m_vertexNormals[i] = glm::normalize(pointCoord - self_center);

		// Have a very, very slight luminous intensity drop off the further out we go
		// Real-life has this set not at 0.375, but 2
		// However setting that value to 2 for our model makes things look way too dark
		// Also reminder: 1.33203125 = 10^0.125; theoretically we could do smth like pow(10/glm::length(lightVector), 0.125)
		// But that feels like overkill for something I can manually change if needed
		glm::vec3 lightVector = sun_center - pointCoord;
		lightVector = 1.33203125f * glm::normalize(lightVector) / (float)pow(glm::length(lightVector), 0.125);

		m_vertexLight[i] = lightVector;
	}

	glBindVertexArray(m_vao);
	updateRendering(m_vertexPositions, &m_posVbo);
	updateRendering(m_vertexNormals, &m_normVbo);
	updateRendering(m_vertexLight, &m_lightVbo);
	glBindVertexArray(0); // Unbinding
}

glm::vec3 Mesh::getSelfCenter() const
{
	return self_center;
}