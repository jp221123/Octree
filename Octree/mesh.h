#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>

#include <random>
#include <vector>
#include <array>

class Mesh {
protected:
    int numTriangles{ 0 };
    GLuint vertexArrayID{ 0 };
    GLuint vertexBufferID{ 0 };
    GLuint colorBufferID{ 0 };
    GLuint normalBufferID{ 0 };
    GLuint elementBufferID{ 0 };

    std::vector<GLfloat> vertices;
    std::vector<GLfloat> colors;
    std::vector<GLfloat> normals;
    std::vector<unsigned int> indices;

public:
    Mesh() {}
    Mesh(const Mesh& other);
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(const Mesh& other);
    Mesh& operator=(Mesh&& other) noexcept;
    virtual ~Mesh();
    GLuint getVertexArrayID() { return vertexArrayID; }
    int getNumTriangles() { return numTriangles; }
protected:
    virtual void generateMesh(std::mt19937& rng) = 0;
    void bind();
    void generateFromTriangles(const std::vector<GLfloat>& triangles);
private:
    void addVertex(const std::array<GLfloat, 3>& vertex);
    void addColor(const std::array<GLfloat, 3>& color);
    void addNormal(const glm::vec3& normal, int index);
    void addIndex(int index);
};