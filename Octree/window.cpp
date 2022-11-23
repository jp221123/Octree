#include "window.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

Window::Window(GLFWwindow* window):
	glfwWindow(window) {
	updateMatrix();
}

void Window::updateMatrix(){
	// Projection matrix : 45¡Æ Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	projMat = glm::perspective(glm::radians(fov), (float)width / (float)height, 0.1f, 100.0f);

	// Or, for an ortho camera :
	// glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates
}