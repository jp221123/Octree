#pragma once
#include <glm/glm.hpp>

#include "window.h"

class Camera {
private:
	glm::mat4 viewMat;
public:
	glm::vec3 position;
	glm::vec3 direction, right, up;
	
	float horizontalAngle = 3.14f;
	float verticalAngle = 0.0f;

	Camera(glm::vec3 position = glm::vec3(0, 0, 5));
	glm::mat4 getViewMat() {
		return viewMat;
	}
	void updateMatrix();
	void updateDirection();
	void updatePosition(Window& window, double time);
};
