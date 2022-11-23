#include "mesh.h"

#include <map>
void Mesh::generateFromTriangles(const std::vector<GLfloat>& triangles) {
    numTriangles = triangles.size() / 9;

    std::map<std::array<GLfloat, 3>, int> vertexToIndex;
    int vertexIndex = 0;

    std::map<std::array<GLfloat, 3>, std::vector<int>> vertexToIncidentTriangle;
    int triangleIndex = 0;
    std::vector<glm::vec3> triangleToNormal;
    std::vector<GLfloat> triangleToArea;

    for (int triangleIndex = 0; triangleIndex < numTriangles; triangleIndex++) {
        std::array<glm::vec3, 3> triangle;
        for (int i = 0; i < 3; i++){
            std::array<GLfloat, 3> vertex = {
                triangles[triangleIndex*9 + i*3],
                triangles[triangleIndex*9 + i*3 +1],
                triangles[triangleIndex*9 + i*3 + 2]
            };
            if (!vertexToIndex.count(vertex)) {
                vertexToIndex[vertex] = vertexIndex++;
                addVertex(vertex);
            }
            addIndex(vertexToIndex[vertex]);
            vertexToIncidentTriangle[vertex].push_back(triangleIndex);
            triangle[i] = { vertex[0], vertex[1], vertex[2] };
        }

        glm::vec3 e12 = triangle[1] - triangle[0];
        glm::vec3 e13 = triangle[2] - triangle[0];
        glm::vec3 normal = glm::cross(e12, e13);
        GLfloat area = glm::length(normal);
        triangleToArea.push_back(area);
        constexpr float EPS = 0.000'001f;
        if (area > EPS)
            triangleToNormal.push_back(glm::normalize(normal));
        else
            triangleToNormal.push_back(glm::vec3(0.0f));
    }

    normals.resize(vertexIndex*3);
    for (auto& entry : vertexToIncidentTriangle) {
        auto& v = entry.first;
        int vertexIndex = vertexToIndex[v];
        glm::vec3 normal(0.0f);
        for (auto& triangleIndex : entry.second) {
            GLfloat tArea = triangleToArea[triangleIndex];
            glm::vec3 tNormal = triangleToNormal[triangleIndex];
            normal += tNormal * tArea;
        }
        addNormal(normal, vertexIndex);
    }
}
void Mesh::addVertex(const std::array<GLfloat, 3>& vertex) {
    for (int i = 0; i < 3; i++)
        vertices.push_back(vertex[i]);
}
void Mesh::addColor(const std::array<GLfloat, 3>& color) {
    for (int i = 0; i < 3; i++)
        colors.push_back(color[i]);
}
void Mesh::addNormal(const glm::vec3& normal, int index) {
    for (int i = 0; i < 3; i++)
        normals[index * 3 + i] = normal[i];
}
void Mesh::addIndex(int index) {
    indices.push_back(index);
}

void Mesh::bind() {
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                  // attribute #
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    //glGenBuffers(1, &colorBufferID);
    //glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
    //glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(
    //    1,                                // attribute #
    //    3,                                // size
    //    GL_FLOAT,                         // type
    //    GL_FALSE,                         // normalized?
    //    0,                                // stride
    //    (void*)0                          // array buffer offset
    //);

    glGenBuffers(1, &normalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,                                // attribute #
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    glGenBuffers(1, &elementBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

Mesh::Mesh(Mesh&& other) noexcept {
    numTriangles = other.numTriangles;
    vertices = std::move(other.vertices);
    colors = std::move(other.colors);
    normals = std::move(other.normals);
    indices = std::move(other.indices);
    if (vertexArrayID == other.vertexArrayID)
        other.vertexArrayID = 0;
    else
        std::swap(vertexArrayID, other.vertexArrayID);
    if (vertexBufferID == other.vertexBufferID)
        other.vertexBufferID = 0;
    else
        std::swap(vertexBufferID, other.vertexBufferID);
    if (colorBufferID == other.colorBufferID)
        other.colorBufferID = 0;
    else
        std::swap(colorBufferID, other.colorBufferID);
    if (normalBufferID == other.normalBufferID)
        other.normalBufferID = 0;
    else
        std::swap(normalBufferID, other.normalBufferID);
    if (elementBufferID == other.elementBufferID)
        other.elementBufferID = 0;
    else
        std::swap(elementBufferID, other.elementBufferID);
}
Mesh& Mesh::operator=(Mesh&& other) noexcept {
    numTriangles = other.numTriangles;
    vertices = std::move(other.vertices);
    colors = std::move(other.colors);
    normals = std::move(other.normals);
    indices = std::move(other.indices);
    if (vertexArrayID == other.vertexArrayID)
        other.vertexArrayID = 0;
    else
        std::swap(vertexArrayID, other.vertexArrayID);
    if (vertexBufferID == other.vertexBufferID)
        other.vertexBufferID = 0;
    else
        std::swap(vertexBufferID, other.vertexBufferID);
    if (colorBufferID == other.colorBufferID)
        other.colorBufferID = 0;
    else
        std::swap(colorBufferID, other.colorBufferID);
    if (normalBufferID == other.normalBufferID)
        other.normalBufferID = 0;
    else
        std::swap(normalBufferID, other.normalBufferID);
    if (elementBufferID == other.elementBufferID)
        other.elementBufferID = 0;
    else
        std::swap(elementBufferID, other.elementBufferID);
    return *this;
}

Mesh::Mesh(const Mesh& other) {
    numTriangles = other.numTriangles;
    vertices = other.vertices;
    colors = other.colors;
    normals = other.normals;
    indices = other.indices;
    bind();
}

Mesh& Mesh::operator=(const Mesh& other) {
    numTriangles = other.numTriangles;
    vertices = other.vertices;
    colors = other.colors;
    normals = other.normals;
    indices = other.indices;
    bind();
    return *this;
}

Mesh::~Mesh() {
    if (vertexArrayID != 0)
        glDeleteVertexArrays(1, &vertexArrayID);
    if (vertexBufferID != 0)
        glDeleteBuffers(1, &vertexBufferID);
    if (colorBufferID != 0)
        glDeleteBuffers(1, &colorBufferID);
    if (normalBufferID != 0)
        glDeleteBuffers(1, &normalBufferID);
    if (elementBufferID != 0)
        glDeleteBuffers(1, &elementBufferID);
}