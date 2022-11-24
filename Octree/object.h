#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>

#include "mesh.h"
#include "shader.h"
#include "window.h"
#include "camera.h"

#include <random>
struct Box {
    std::array<float, 3> mins;
    std::array<float, 3> maxs;
    Box() {}
    Box(float x1, float y1, float z1, float x2, float y2, float z2)
        : mins{ x1, y1, z1 }, maxs{ x2, y2, z2 }{}
    Box(const std::array<float, 3>& mins, const std::array<float, 3>& maxs)
        : mins(mins), maxs(maxs) {}
    Box(const Box& box)
        : mins(box.mins), maxs(box.maxs) {}
    std::array<float, 3> getCenter() const;
};

enum class SolidBodyType {
    SPHERE,
    CUBE,
};

class SolidBody {
protected:
    const SolidBodyType classType;
    int stateIndex{ 0 };
    std::array<glm::mat4, 2> model{ glm::mat4(1.0f),  };
    // assume all scales are done before rotations, and then translations
    std::array<glm::vec3, 2> worldPos{ glm::vec3(0.0f),}; // of center
    std::array<float, 2> scaledFactor{ 1.0f, };

    int numTriangles;
	GLuint vertexArrayID;
    glm::vec3 ambientColor, diffuseColor, specularColor;
    float shininess;
    bool isDirty{ true };
public:
    SolidBody(Mesh& mesh, std::mt19937& rng, SolidBodyType classType);

    bool isClicked{ false };
    const glm::mat4& modelMatrix() const { return model[stateIndex]; }
    void updatePosition(Window& window, const Camera& camera, double t);
    void revert();

    void draw(const SolidBodyShader& shader, const glm::mat4& projMat, const glm::mat4& viewMat);

    void scale(float s);
    void translate(const glm::vec3& t);
    //void update();
    //void rotate(glm::vec3 axis, float degrees);

    // for safety, overestimate the boundary of the object
    bool intersects(const SolidBody* other, const float MARGIN = 0.01f);
    // for safety, underestimate the boundary of the object
    bool intersects(const Box& box, const float MARGIN = -0.000'01f);
    // for safety, overestimate the boundary of the object
    bool containedInBoundary(const Box& box, const float MARGIN = 0.01f);

    // if returns yes, the intersection is from + [t1Out...t2Out] * (to-from)
    bool intersects(const glm::vec3& from, const glm::vec3& to, float& t1Out, float& t2Out);
    static bool intersects(const Box& box, const glm::vec3& from, const glm::vec3& to, float& t1Out, float& t2Out);
private:
    void update();
    void makeColor(std::mt19937& rng);

    friend std::ostream& operator<<(std::ostream& os, const SolidBody&);
};

std::ostream& operator<<(std::ostream& os, const SolidBody&);