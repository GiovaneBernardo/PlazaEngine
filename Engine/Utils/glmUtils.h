#pragma once
#ifndef GLM_UTILS_H
#define GLM_UTILS_H

#include "ThirdParty/glm/glm.hpp"
#include "ThirdParty/glm/gtc/matrix_transform.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

namespace glm {
	inline glm::vec3 eulerToRadians(glm::vec3 euler) {
		return glm::vec3(glm::radians(euler.x), glm::radians(euler.y), glm::radians(euler.z));
	}

	inline bool isInside(glm::vec2 vec, glm::vec2 startVec, glm::vec2 endVec) {
		return vec.x >= startVec.x && vec.x <= endVec.x && vec.y >= startVec.y && vec.y <= endVec.y;
	}
} // namespace glm
#endif
