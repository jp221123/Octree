#include "window.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "camera.h"

#include <iostream>

void Window::init(GLFWwindow* window){
	glfwWindow = window;
	updateMatrix();
}

glm::vec3 Window::screenToWorld(int x, int y, const glm::mat4& invMat) {
	glm::vec4 ndc{ 2.0f*(float)x / width - 1.0f, 1.0f - 2.0f*(float)y / height, 0, 1.0 };
	glm::vec4 pos = invMat * ndc;
	return pos / pos.w;
}

std::array<glm::vec3, 2> Window::pointToWorld(int x, int y, const Camera& camera) {
	glm::mat4 vpMat = getProjMat() * camera.getViewMat();
	glm::mat4 invMat = glm::inverse(vpMat);

	glm::vec3 from = camera.getPosition();
	glm::vec3 to = screenToWorld(x, y, invMat);
	// near - from = (to - from) * k
	// |(near - from) projected to direction| = camera.near
	float proj = glm::dot(to - from, glm::normalize(camera.getDirection()));
	float kNear = NEAR / proj;
	float kFar = FAR / proj;
	return { from + (to - from) * kNear, from + (to - from) * kFar };
}

std::array<glm::vec3, 4> Window::rectToWorld(int x1, int x2, int y1, int y2, const Camera& camera) {
	glm::mat4 vpMat = getProjMat() * camera.getViewMat();
	glm::mat4 invMat = glm::inverse(vpMat);

	std::array<int, 2> x{ x1, x2 };
	std::array<int, 2> y{ y1, y2 };
	std::array<glm::vec3, 4> res;
	for (int mask = 0; mask < (1 << 2); mask++) {
		std::array<int, 2> xy;
		for (int pos = 0; pos < 2; pos++)
			xy[pos] = mask & (1 << pos) ? x[pos] : y[pos];
		res[mask] = screenToWorld(xy[0], xy[1], invMat);
	}
	return res;
}

void Window::updateMatrix(){
	projMat = glm::perspective(
		glm::radians(fov), // fov
		(float)width / (float)height, // aspect
		NEAR, // near
		FAR); // far

	// for an ortho camera :
	// glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f);
}