#include "object.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

SolidBody::SolidBody(Mesh& mesh, std::mt19937& rng)
    : vertexArrayID(mesh.getVertexArrayID()), numTriangles(mesh.getNumTriangles()) {
    makeColor(rng);
}

void SolidBody::makeColor(std::mt19937& rng){
    std::uniform_real_distribution<float> distribution(0.0, 1.0);
    glm::vec3 color;
    for (int i = 0; i < 3; i++)
        color[i] = distribution(rng);

    ambientColor = color * 0.2f;
    diffuseColor = color * 0.8f;
    specularColor = color;

    distribution = std::uniform_real_distribution<float>(5.0, 15.0);
    shininess = distribution(rng);
}

void SolidBody::revert(){
    stateIndex = 1 - stateIndex;
}

// not used for now
//void SolidBody::update() {
//    // same as worldPos = glm::vec3(model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
//    worldPos = glm::vec3(model[3]);
//    // scaledFactor = glm::vec3(glm::length(model[0]), glm::length(model[1]), glm::length(model[2]))
//    // assuming only regular scalings
//    scaledFactor = model[0][0];
//}

// assuming we are doing scale -> translate -> translate -> ... only
void SolidBody::scale(float s) {
    model[1-stateIndex] = glm::scale(glm::vec3(s, s, s)) * model[stateIndex];
    scaledFactor[1-stateIndex] = s * scaledFactor[stateIndex];
    worldPos[1 - stateIndex] = worldPos[stateIndex];
    stateIndex = 1 - stateIndex;
}

void SolidBody::translate(glm::vec3 t) {
    model[1-stateIndex] = glm::translate(t) * model[stateIndex];
    scaledFactor[1 - stateIndex] = scaledFactor[stateIndex];
    worldPos[1-stateIndex] = worldPos[stateIndex] + t;
    stateIndex = 1 - stateIndex;
}

// not used for now
//void SolidBody::rotate(glm::vec3 axis, float degrees) {
//    model = glm::rotate(glm::radians(degrees), axis) * model;
//}

void SolidBody::draw(const SolidBodyShader& shader, const glm::mat4& projMat, const glm::mat4& viewMat) {
    glUseProgram(shader.programID);

    glm::mat4 mvMat = viewMat * model[stateIndex];
    glm::mat4 normalMat = glm::transpose(glm::inverse(mvMat));
    glUniformMatrix4fv(shader.pMatID, 1, GL_FALSE, &projMat[0][0]);
    glUniformMatrix4fv(shader.mvMatID, 1, GL_FALSE, &mvMat[0][0]);
    glUniformMatrix4fv(shader.normalMatID, 1, GL_FALSE, &normalMat[0][0]);

    glUniform1f(shader.shininessID, shininess);

    glUniform3fv(shader.ambientColorID, 1, &ambientColor[0]);
    glUniform3fv(shader.diffuseColorID, 1, &diffuseColor[0]);
    glUniform3fv(shader.specularColorID, 1, &specularColor[0]);

    glBindVertexArray(vertexArrayID);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL).
    glDrawElements(
        GL_TRIANGLES,      // mode
        numTriangles*3,    // count
        GL_UNSIGNED_INT,   // type
        (void*)0         // element array buffer offset
    );

    glUseProgram(0);
    glBindVertexArray(0);
}

void SolidBody::updatePosition(Window& window, Camera& camera, double t) {
    constexpr float speed = 0.5f;
    const float dist = glm::length(worldPos[stateIndex] - camera.position);
    const float sd = speed * dist;

    glm::vec3 trans(0.0f);
    auto updatePositionKey = [&](int key, glm::vec3&& base) {
        if (window.isKeyPressed[key]) {
            float dt = t - window.tKey[key];
            trans += base * dt;
        }
    };

    updatePositionKey(GLFW_KEY_W, camera.up*sd);
    updatePositionKey(GLFW_KEY_S, camera.up*(-sd));
    updatePositionKey(GLFW_KEY_A, camera.right*(-sd));
    updatePositionKey(GLFW_KEY_D, camera.right*sd);

    translate(trans);
}