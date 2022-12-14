#pragma once

#define GLEW_STATIC
#include <gl/glew.h>
#include <GLFW/glfw3.h>

int main();

int initN();
int initGL();
int initGLSL();
int initObject(int);
void makeObjects(int);
int initMisc();

void update();
void display();
void record();
void clean();

//void window_size_callback(GLFWwindow* window, int newWidth, int newHeight);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);