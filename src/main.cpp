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
#include "sphere.h"
#include "cube.h"
#include "octree.h"

#include <iostream>
#include <random>
#include <chrono>
#include <set>

SolidBodyShader shader;
Window window;
Camera camera;
std::mt19937 rng;
CubeMesh cubeMesh;
TriangleMesh triangleMesh;
std::vector<SphereMesh> sphereMesh;

constexpr bool toRecord = false;
FILE* ffmpeg;
std::vector<int> ffmpegBuffer;

constexpr float MAX_COORDINATE = 10.0f;
Octree octree(MAX_COORDINATE);
std::vector<std::unique_ptr<SolidBody>> objects;
std::set<SolidBody*> clickedObjects;

int main() {
    int N = initN();

    if (!initGL())
        return -1;

    if (!initGLSL())
        return -1;

    if (!initMisc())
        return -1;

    initObject(N);

    while(!glfwWindowShouldClose(window.glfwWindow) && !window.toClose){
        update();
        display();
        if(toRecord)
            record();
        glfwPollEvents();
    }

    clean();

    return 0;
}

int initN() {
    std::cout << "Octree node capacity = " << OctreeNode::CAPACITY << '\n' << std::endl;

    std::cout
        << "Usage\n"
        << "Enter: toggle the random movements of objects\n"
        << "Left click [with L-shift]: select [add] an object\n"
        << "WASD buttons: move the seleted objects\n"
        << "O button: toggle the structure of the octree\n"
        << "Space: reset the camera\n"
        << "Arrow buttons: move the camera\n"
        << "Right click drag: rotate the camera\n"
        << "ESC: exit the program\n"
        << std::endl;
    constexpr int N_MIN = 1;
    constexpr int N_MAX = 50000;
    std::cout << "Enter the number of objects (1 ~ 50000)" << std::endl;
    int N;
    std::cin >> N;
    std::cout << std::endl;
    N = std::min(N, N_MAX);
    N = std::max(N, N_MIN);

    return N;
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

    GLFWwindow* glfwWindow = glfwCreateWindow(window.width, window.height, "Octree Demo", NULL, NULL);
    if (glfwWindow == NULL) {
        std::cerr << "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials" << std::endl;
        glfwTerminate();
        return false;
    }
    window.init(glfwWindow);
    glfwGetFramebufferSize(glfwWindow, &window.width, &window.height);

    glfwMakeContextCurrent(window.glfwWindow);

    // glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwSetInputMode(window.glfwWindow, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window.glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    //glfwSetWindowSizeCallback(window.glfwWindow, window_size_callback);
    glfwSetFramebufferSizeCallback(window.glfwWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(window.glfwWindow, cursor_position_callback);
    glfwSetKeyCallback(window.glfwWindow, key_callback);
    glfwSetMouseButtonCallback(window.glfwWindow, mouse_button_callback);
    glfwSetScrollCallback(window.glfwWindow, scroll_callback);

    // glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    return true;
}

int initGLSL() {
    shader.init();
    octree.init();

    return true;
}

int initObject(int N) {
    constexpr int MAX_SUBDIVISION = 4;
    rng = std::mt19937(std::chrono::steady_clock::now().time_since_epoch().count());
    cubeMesh = CubeMesh(rng);
    triangleMesh = TriangleMesh(rng);
    for(int i=0; i<=MAX_SUBDIVISION; i++)
        sphereMesh.push_back(SphereMesh(rng, i));

    makeObjects(N);

    return true;
}

int initMisc() {
    if (toRecord) {
        const char* FFMPEG_PATH = "..\\lib\\ffmpeg-5.1.2-essentials_build\\bin\\ffmpeg.exe "
            "-y -f rawvideo -pix_fmt rgba -s 1024x768 -i - -y -pix_fmt yuv420p -crf 10 -vf vflip ..\\demo\\demo.mp4";
        ffmpeg = _popen(FFMPEG_PATH, "wb");
        ffmpegBuffer.resize(window.width * window.height);
    }

    return true;
}

void makeObjects(int N) {
    constexpr float MIN_SCALE = 0.001;
    constexpr float MAX_SCALE = 1.0;

    std::uniform_real_distribution<float> rDist(MIN_SCALE, MAX_SCALE);
    std::uniform_real_distribution<float> tDist(-MAX_COORDINATE + MAX_SCALE * 2, MAX_COORDINATE - MAX_SCALE * 2);
    std::uniform_int_distribution<int> dist(0, 1);

    for (int i = 0; i < N; i++) {
        while (true) {
            int type = dist(rng);
            float scale = rDist(rng);
            // cube -> 2*2*2 = 8
            // sphere -> 4/3pi ~ 4
            if (type == 1)
                scale /= 1.25f;
            glm::vec3 trans = { tDist(rng), tDist(rng), tDist(rng) };
            int subdivision = 4;
            {
                float r = 0.5f;
                while (scale < r && subdivision > 0) {
                    r /= 3;
                    subdivision--;
                }
            }

            if (type == 0)
                objects.push_back(std::make_unique<Sphere>(sphereMesh[subdivision], rng));
            else
                objects.push_back(std::make_unique<Cube>(cubeMesh, rng));
            auto object = objects.back().get();
            object->scale(scale);
            object->translate(trans);

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

}

void update() {
    const double t = glfwGetTime();

    camera.move(window, t);

    camera.updateMatrix();
    window.updateMatrix();

    // an object moves only if it can move, that is,
    // i) the object does not go out of the boundary
    // ii) the object does not collide with other objects

    auto move = [&](SolidBody* object) {
        object->updatePosition(window, camera, t);
        if (!octree.update(object)) {
            object->revert();
            object->makeRandomMovingDirection(rng);
        }
    };

    for (auto& object : objects)
        move(object.get());

    for (int key : {GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D})
        window.tKey[key] = t;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // std::cout << clickedObjects.size() << std::endl;

    for (auto& object : objects)
        object->draw(shader, window.getProjMat(), camera.getViewMat());

    if(window.drawsOctree)
        octree.draw(window.getProjMat(), camera.getViewMat());

    glfwSwapBuffers(window.glfwWindow);
}

void record() {
    glReadPixels(0, 0, window.width, window.height, GL_RGBA, GL_UNSIGNED_BYTE, ffmpegBuffer.data());
    fwrite(ffmpegBuffer.data(), ffmpegBuffer.size() * sizeof(int), 1, ffmpeg);
}

void clean() {
    glfwTerminate();
    if (toRecord)
        std::cout << _pclose(ffmpeg) << std::endl;
}


void framebuffer_size_callback(GLFWwindow* glfwWindow, int newWidth, int newHeight){
    // std::cout << width << '*' << height << "->" << newWidth << '*' << newHeight << std::endl;
    window.width = newWidth;
    window.height = newHeight;
    glViewport(0, 0, window.width, window.height);
    ffmpegBuffer.resize(window.width * window.height);
}

void mouse_button_callback(GLFWwindow* glfwWindow, int button, int action, int mods){
    bool isPressed;
    if (action == GLFW_PRESS)
        isPressed = true;
    else if (action == GLFW_RELEASE)
        isPressed = false;
    else
        return;

    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        window.isLeftMousePressed = isPressed;
        if (isPressed) {
            auto [near, far] = window.pointToWorld(window.cursorX, window.cursorY, camera);
            SolidBody* obj = octree.rayQuery(near, far);

            if (obj == nullptr) {
                if (window.isKeyPressed[GLFW_KEY_LEFT_SHIFT]) {
                    // do nothing
                }
                else{
                    // release all clicked objects
                    for (auto object : clickedObjects)
                        object->isClicked = false;
                    clickedObjects.clear();
                }
            }
            else {
                if (window.isKeyPressed[GLFW_KEY_LEFT_SHIFT]) {
                    if (clickedObjects.count(obj)) {
                        obj->isClicked = false;
                        clickedObjects.erase(obj);
                    }
                    else {
                        obj->isClicked = true;
                        clickedObjects.insert(obj);
                    }
                }
                else {
                    for (auto object : clickedObjects)
                        object->isClicked = false;
                    clickedObjects.clear();
                    obj->isClicked = true;
                    clickedObjects.insert(obj);
                }
            }
        }
        //  todo: implement range frustum query when dragging
        //  need to find out clicked objects
        //  clickedObjects = octree.frustumQuery(
        //    camera.getPosition(),
        //    window.rectToWorld(window.cursorX, window.cursorX2, window.cursorY, window.cursorY2, camera),
        //    window.NEAR,
        //    window.FAR);
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        window.isRightMousePressed = isPressed;
        break;
    }
}

void cursor_position_callback(GLFWwindow* glfwWindow, double x, double y) {
    // glfwGetCursorPos(window, &xpos, &ypos);
    if(!window.isLeftMousePressed && !window.isRightMousePressed) {
        window.cursorX = x;
        window.cursorY = y;
        return;
    }

    if (window.isLeftMousePressed) {
        window.cursorX2 = x;
        window.cursorY2 = y;
    }
    else { // window.isRightMousePressed
        float dx = x - window.cursorX;
        float dy = y - window.cursorY;
        window.cursorX = x;
        window.cursorY = y;

        // Compute the new orientation
        constexpr float speed = 0.001f;
        camera.move(speed * dx, speed * dy);
    }
}

void key_callback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods) {
    double t = glfwGetTime();

    if (action == GLFW_PRESS && key == GLFW_KEY_O)
        window.drawsOctree ^= true;

    if (action == GLFW_PRESS && key == GLFW_KEY_ENTER)
        window.randomMoves ^= true;

    if (action == GLFW_PRESS && key == GLFW_KEY_SPACE)
        camera.reset();
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        window.toClose = true;

    bool isPressed;
    if (action == GLFW_RELEASE)
        isPressed = false;
    else if (action == GLFW_PRESS)
        isPressed = true;
    else
        return;
    window.isKeyPressed[key] = isPressed;
    window.tKey[key] = t;

    //if (action == GLFW_PRESS && key == GLFW_KEY_SPACE) {
    //    twoObjectTest();
    //}
}

void scroll_callback(GLFWwindow* glfwWindow, double xoffset, double yoffset) {
    float& fov = window.fov;
    fov += yoffset;
    fov = std::max(fov, 30.0f);
    fov = std::min(fov, 60.0f);

    //std::cout << xoffset << ' ' << yoffset << std::endl;
    //std::cout << fov << std::endl;
}