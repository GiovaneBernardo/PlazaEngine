#pragma once
#ifndef header_H
#define header_H
#include <glm/glm.hpp>
// Struct of all aplication gui sizes
struct AppSizes {
	glm::vec2 appSize = { 2560, 1080 };
	glm::vec2 sceneSize = { 1920, 720 };
	glm::vec2 hierarchySize = { 500, 1000 };
	glm::vec2 inspectorSize = { 500, 1000 };
};
#endif // header_H