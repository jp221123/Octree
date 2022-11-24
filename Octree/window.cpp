#include "window.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

Window::Window(GLFWwindow* window):
	glfwWindow(window) {
	updateMatrix();
}

void Window::updateMatrix(){
	projMat = glm::perspective(
		glm::radians(fov), // fov
		(float)width / (float)height, // aspect
		0.1f, // near
		100.0f); // far

	// for an ortho camera :
	// glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f);
}