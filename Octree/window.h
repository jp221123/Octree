#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <unordered_map>
class Window {
private:
	glm::mat4 projMat;
public:
	GLFWwindow* glfwWindow;
	float fov = 45.0f;
	int width = 1024;
	int height = 768;

	bool drawsOctree = true;

	double cursorX, cursorY;
	bool isMousePressed = false;

	std::unordered_map<int, bool> isKeyPressed;
	std::unordered_map<int, double> tKey;

	Window(GLFWwindow*);
	glm::mat4 getProjMat() {
		return projMat;
	}
	void updateMatrix();
};