#include "mesh_triangle.h"

TriangleMesh::TriangleMesh(std::mt19937& rng) {
    vertices.clear();
    colors.clear();
    normals.clear();
    indices.clear();

    generateMesh(rng);
    bind();
}

void TriangleMesh::generateMesh(std::mt19937& rng){
    std::vector<GLfloat> triangles = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         0.0f,  1.0f, 0.0f,
    };

    generateFromTriangles(triangles);

    std::uniform_real_distribution<float> distribution(0.0, 1.0);

    int n = vertices.size();
    for (int i = 0; i < n; i++)
        colors.push_back(distribution(rng));
}