#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <unordered_map>
#include <array>

class Camera;
class Window {
private:
	glm::mat4 projMat;
public:
	GLFWwindow* glfwWindow;
	float fov = 45.0f;
	int width = 1024;
	int height = 768;
	const float NEAR = 0.1f;
	const float FAR = 100.0f;

	bool randomMoves{ false };
	bool drawsOctree{ true };

	double cursorX, cursorY;
	// unused for now, need for dragging
	double cursorX2, cursorY2;
	bool isLeftMousePressed{ false };
	bool isRightMousePressed{ false };

	std::unordered_map<int, bool> isKeyPressed;
	std::unordered_map<int, double> tKey;

	void init(GLFWwindow* window);

	glm::mat4 getProjMat() { return projMat; }
	void updateMatrix();

	// returns [near, far]
	std::array<glm::vec3, 2> pointToWorld(int x, int y, const Camera& camera);
	std::array<glm::vec3, 4> rectToWorld(int x1, int x2, int y1, int y2, const Camera& camera);
private:
	glm::vec3 screenToWorld(int x, int y, const glm::mat4& invMat);
};