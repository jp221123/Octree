#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>

#include "mesh.h"
#include "shader.h"
#include "window.h"
#include "camera.h"

#include <random>

class SolidBody {
protected:
    int stateIndex{ 0 };
    std::array<glm::mat4, 2> model{ glm::mat4(1.0f),  };
    // assume all scales are done before rotations, and then translations
    std::array<glm::vec3, 2> worldPos{ glm::vec3(0.0f),}; // of center
    std::array<float, 2> scaledFactor{ 1.0f, };

    int numTriangles;
	GLuint vertexArrayID;
    glm::vec3 ambientColor, diffuseColor, specularColor;
    float shininess;
public:
    SolidBody(Mesh& mesh, std::mt19937& rng);

    const glm::mat4 modelMatrix() const { return model[stateIndex]; }
    const glm::vec3 getWorldPosition() const { return worldPos[stateIndex]; }
    void updatePosition(Window& window, Camera& camera, double t);
    void revert();

    void draw(const SolidBodyShader& shader, const glm::mat4& projMat, const glm::mat4& viewMat);

    void scale(float s);
    void translate(glm::vec3 t);
    //void update();
    //void rotate(glm::vec3 axis, float degrees);
private:
    void update();
    void makeColor(std::mt19937& rng);
};