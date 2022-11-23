#include "main.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "window.h"
#include "camera.h"
#include "shader.h"
#include "mesh_sphere.h"
#include "mesh_cube.h"
#include "mesh_triangle.h"
#include "object.h"
#include "octree.h"

#include <iostream>
#include <random>
#include <chrono>

SolidBodyShader shader;
Window window(nullptr);
Camera camera;
std::mt19937 rng;
CubeMesh cubeMesh;
TriangleMesh triangleMesh;
std::vector<SphereMesh> sphereMesh;

Octree octree(10.0);
std::vector<std::unique_ptr<SolidBody>> objects;
int main() {
    if (!initGL())
        return -1;

    if (!initGLSL())
        return -1;

    if (!initMisc())
        return -1;

    initObject();

    do {
        update();
        display();
    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window.glfwWindow, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window.glfwWindow) == 0);

    clean();
    return 0;
}

int initGL() {
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    GLFWwindow* glfwWindow = glfwCreateWindow(window.width, window.height, "Octree Demo", NULL, NULL);
    if (glfwWindow == NULL) {
        std::cerr << "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials" << std::endl;
        glfwTerminate();
        return false;
    }
    window = Window(glfwWindow);
    glfwGetFramebufferSize(glfwWindow, &window.width, &window.height);

    glfwMakeContextCurrent(window.glfwWindow); // Initialize GLEW

    // glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return false;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window.glfwWindow, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window.glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    //glfwSetWindowSizeCallback(window.glfwWindow, window_size_callback);
    glfwSetFramebufferSizeCallback(window.glfwWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(window.glfwWindow, cursor_position_callback);
    glfwSetKeyCallback(window.glfwWindow, key_callback);
    glfwSetMouseButtonCallback(window.glfwWindow, mouse_button_callback);
    glfwSetScrollCallback(window.glfwWindow, scroll_callback);

    // glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    return true;
}

int initGLSL() {
    // Create and compile our GLSL program from the shaders
    shader.init();
    octree.init();

    return true;
}

int initObject() {
    constexpr int MAX_SUBDIVISION = 4;
    rng = std::mt19937(std::chrono::steady_clock::now().time_since_epoch().count());
    cubeMesh = CubeMesh(rng);
    triangleMesh = TriangleMesh(rng);
    for(int i=0; i<=MAX_SUBDIVISION; i++)
        sphereMesh.push_back(SphereMesh(rng, i));

    constexpr int N = 10000;
    std::uniform_real_distribution<float> rDist(0.01, 0.2);
    std::uniform_real_distribution<float> tDist(-5.0, 5.0);
    for (int i = 0; i < N; i++) {
        while (true) {
            objects.push_back(std::make_unique<Sphere>(sphereMesh[3], rng));
            Sphere* object = (Sphere*)objects.back().get();
            object->scale(rDist(rng));
            object->translate({ tDist(rng), tDist(rng), tDist(rng) });

            // brute-force test
            //bool ok = true;
            //for (int j = 0; j < i; j++) {
            //    if (object->intersects((Sphere*)objects[j].get())) {
            //        ok = false;
            //        break;
            //    }
            //}
            //if (!ok)
            //    continue;

            if (octree.insert(object))
                break;
            else
                objects.pop_back();
        }
    }

    return true;
}

int initMisc() {


    return true;
}

void update() {
    const double t = glfwGetTime();

    camera.updateDirection();
    camera.updatePosition(window, t);

    camera.updateMatrix();
    window.updateMatrix();

    // an object moves only if it can move, that is,
    // i) the object does not go out of the boundary
    // ii) the object does not colide with other objects
    const auto& clickedObjects = objects;
    for (auto& object : clickedObjects) {
        auto sphere = (Sphere*)object.get();

        octree.remove(sphere);
        sphere->updatePosition(window, camera, t);
        if (!octree.insert(sphere)) {
            sphere->revert();
            if (!octree.insert(sphere)) {
                std::cerr << "unreachable" << std::endl;
            }
        }
    }

    for (int key : {GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D})
        window.tKey[key] = t;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto& object : objects)
        object->draw(shader, window.getProjMat(), camera.getViewMat());

    if(window.drawsOctree)
        octree.draw(window.getProjMat(), camera.getViewMat());

    // Swap buffers
    glfwSwapBuffers(window.glfwWindow);
    glfwPollEvents();
}

void clean() {
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}


void framebuffer_size_callback(GLFWwindow* glfwWindow, int newWidth, int newHeight){
    // std::cout << width << '*' << height << "->" << newWidth << '*' << newHeight << std::endl;
    window.width = newWidth;
    window.height = newHeight;
    glViewport(0, 0, window.width, window.height);
}

void mouse_button_callback(GLFWwindow* glfwWindow, int button, int action, int mods){
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        window.isMousePressed = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        window.isMousePressed = false;
}

void cursor_position_callback(GLFWwindow* glfwWindow, double x, double y) {
    // glfwGetCursorPos(window, &xpos, &ypos);
    if(!window.isMousePressed) {
        window.cursorX = x;
        window.cursorY = y;
        return;
    }

    float dx = x - window.cursorX;
    float dy = y - window.cursorY;
    window.cursorX = x;
    window.cursorY = y;

    // Compute new orientation
    constexpr float speed = 0.001f;
    camera.horizontalAngle += speed * dx;
    camera.verticalAngle += speed * dy;
}

void key_callback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods) {
    double t = glfwGetTime();
    bool isPressed;
    if (action == GLFW_RELEASE)
        isPressed = false;
    else if (action == GLFW_PRESS)
        isPressed = true;
    else
        return;

    if (action == GLFW_PRESS && key == GLFW_KEY_ENTER)
        window.drawsOctree ^= true;

    window.isKeyPressed[key] = isPressed;
    window.tKey[key] = t;
}

void scroll_callback(GLFWwindow* glfwWindow, double xoffset, double yoffset) {
    float& fov = window.fov;
    fov += yoffset;
    fov = std::max(fov, 30.0f);
    fov = std::min(fov, 60.0f);

    //std::cout << xoffset << ' ' << yoffset << std::endl;
    //std::cout << fov << std::endl;
}