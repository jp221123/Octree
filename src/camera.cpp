#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "window.h"

Camera::Camera(glm::vec3 position):
    position(position){
    updateDirection();
    updateMatrix();
}

void Camera::reset() {
    horizontalAngle = BASE_H_ANGLE;
    verticalAngle = BASE_V_ANGLE;
    position = BASE_POS;
    updateDirection();
}

void Camera::move(float dHorizontalAngle, float dVerticalAngle) {
    horizontalAngle += dHorizontalAngle;
    verticalAngle += dVerticalAngle;
    updateDirection();
}

void Camera::move(Window& window, double t) {
    constexpr float speed = 3.0f;

    auto updatePositionKey = [&](int key, glm::vec3 base) {
        if (window.isKeyPressed[key]) {
            float dt = t - window.tKey[key];
            window.tKey[key] = t;
            position += base * dt * speed;
        }
    };

    updatePositionKey(GLFW_KEY_UP, up);
    updatePositionKey(GLFW_KEY_DOWN, -up);
    updatePositionKey(GLFW_KEY_LEFT, -right);
    updatePositionKey(GLFW_KEY_RIGHT, right);

    updateDirection();
}

void Camera::updateDirection() {
    direction = glm::vec3(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle)
    );

    right = glm::vec3(
        sin(horizontalAngle - 3.14f / 2.0f),
        0,
        cos(horizontalAngle - 3.14f / 2.0f)
    );

    up = glm::cross(right, direction);
}

void Camera::updateMatrix() {
    viewMat = glm::lookAt(
        position,               // camera position
        position + direction, // look-at position
        up                          // up vector
    );
}