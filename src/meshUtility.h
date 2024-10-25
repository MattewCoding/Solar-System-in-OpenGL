#ifndef INCLUDE_MESHCONSTANTS
#define INCLUDE_MESHCONSTANTS

#include <dep/glm/glm.hpp>
#include <dep/glm/gtc/matrix_transform.hpp>

#define PRINT(x) std::cout << (x)

#define X_ROTATION_VECTOR glm::vec3(1.0, 0.0, 0.0)
#define Y_ROTATION_VECTOR glm::vec3(0.0, 1.0, 0.0)
#define Z_ROTATION_VECTOR glm::vec3(0.0, 0.0, 1.0)
#define ZERO_VECTOR glm::vec3(0.0,0.0,0.0)

class MeshUtility
{
public:
	inline static glm::mat4 rotateAroundAxis(const glm::vec3& axis, float angle)
	{
		glm::vec3 normalizedAxis = glm::normalize(axis);
		return glm::rotate(glm::mat4(1.0f), angle, normalizedAxis);
	}

	inline static glm::mat4 translate(const glm::vec3& position)
	{
		return glm::translate(glm::mat4(1.0f), position);
	}
};

#endif