#pragma once
#include <glm/glm.hpp>

class Window;
class Camera {
private:
	glm::mat4 viewMat;
	glm::vec3 position;
	glm::vec3 direction, right, up;
	float horizontalAngle{ BASE_H_ANGLE };
	float verticalAngle{ BASE_V_ANGLE };
	static constexpr float BASE_H_ANGLE = 3.14f;
	static constexpr float BASE_V_ANGLE = 0.0f;
	static constexpr glm::vec3 BASE_POS{0.0f, 0.0f, 5.0f};
public:
	Camera(glm::vec3 position = BASE_POS);
	
	const glm::mat4& getViewMat() const { return viewMat; }
	const glm::vec3& getPosition() const { return position; }
	const glm::vec3& getDirection() const { return direction; }
	const glm::vec3& getRight() const { return right; }
	const glm::vec3& getUp() const { return up; }

	void updateMatrix();
	void move(float dHorizontalAngle, float dVerticalAngle);
	void move(Window& window, double time);
	void reset();
private:
	void updateDirection();
};
