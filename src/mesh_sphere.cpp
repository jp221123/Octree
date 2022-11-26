#include "mesh_sphere.h"

SphereMesh::SphereMesh(std::mt19937& rng, int subdivision) :
    subdivision(subdivision) {
    vertices.clear();
    colors.clear();
    normals.clear();
    indices.clear();

    generateMesh(rng);
    bind();
}

void addVertices(std::vector<float>& triangles, std::vector<glm::vec3>&& vs) {
    for(auto& v: vs)
        for (int i = 0; i < 3; i++)
            triangles.push_back(v[i]);
}

void addVertices(std::vector<float>& triangles, std::vector<float>& vertices, std::vector<int>&& indices) {
    for(auto index: indices)
        for (int i = 0; i < 3; i++)
            triangles.push_back(vertices[index+i]);
}

// icosphere
// adapted from http://www.songho.ca/opengl/gl_sphere.html#icosphere
void SphereMesh::generateMesh(std::mt19937& rng) {
    std::vector<float> triangles;

    constexpr float RADIUS = 1;
    constexpr float PI = 3.1415926535;
    constexpr float H_ANGLE = PI / 180 * 72;    // 72 degree = 360 / 5
    constexpr float V_ANGLE = 0.4636476090008;  // elevation = arctan(1/2) = 26.565 degree

    std::vector<float> vertices(12*3);            // 12 vertices
    int i1, i2;                             // indices
    float z, xy;                            // coords
    float hAngle1 = -PI / 2 - H_ANGLE / 2;  // start from -126 deg at 2nd row
    float hAngle2 = -PI / 2;                // start from -90 deg at 3rd row

    // the first top vertex (0, 0, r)
    vertices[0] = 0;
    vertices[1] = 0;
    vertices[2] = RADIUS;

    // 10 vertices at 2nd and 3rd rows
    for (int i = 1; i <= 5; ++i)
    {
        i1 = i * 3;         // for 2nd row
        i2 = (i + 5) * 3;   // for 3rd row

        z = RADIUS * sinf(V_ANGLE);             // elevaton
        xy = RADIUS * cosf(V_ANGLE);

        vertices[i1] = xy * cosf(hAngle1);      // x
        vertices[i2] = xy * cosf(hAngle2);
        vertices[i1 + 1] = xy * sinf(hAngle1);  // x
        vertices[i2 + 1] = xy * sinf(hAngle2);
        vertices[i1 + 2] = z;                   // z
        vertices[i2 + 2] = -z;

        // next horizontal angles
        hAngle1 += H_ANGLE;
        hAngle2 += H_ANGLE;
    }
    
    {
        // the last bottom vertex (0, 0, -r)
        i1 = 11 * 3;
        vertices[i1] = 0;
        vertices[i1 + 1] = 0;
        vertices[i1 + 2] = -RADIUS;
    }

    // compute and add 20 tiangles of icosahedron first
    int i0 = 0;       // 1st vertex
    int i11 = 11 * 3; // 12th vertex
    for (int i = 1; i <= 5; ++i){
        // 4 vertices in the 2nd row
        int i1 = i * 3;
        int i2;
        if (i < 5)
            i2 = (i + 1) * 3;
        else
            i2 = 3;

        int i3 = (i + 5) * 3;
        int i4;
        if ((i + 5) < 10)
            i4 = (i + 6) * 3;
        else
            i4 = 6 * 3;

        addVertices(triangles, vertices,
                { i0, i1, i2,
                        i1, i3, i2,
                        i2, i3, i4,
                        i3, i11,i4 });
    }

    for(int iter=1; iter <= subdivision; iter++){
        std::vector<float> nextTriangles;

        for (int i = 0; i < triangles.size(); i+=9) {
            // get 3 vertice and texcoords of a triangle
            glm::vec3 v1 = { triangles[i], triangles[i + 1], triangles[i + 2] };
            glm::vec3 v2 = { triangles[i + 3], triangles[i + 4], triangles[i + 5] };
            glm::vec3 v3 = { triangles[i + 6], triangles[i + 7], triangles[i + 8] };

            // get 3 new vertices by spliting half on each edge
            auto u1 = glm::normalize(v1 + v2) * RADIUS;
            auto u2 = glm::normalize(v2 + v3) * RADIUS;
            auto u3 = glm::normalize(v1 + v3) * RADIUS;

            // add 4 new triangles
            addVertices(nextTriangles,
                        { v1, u1, u3,
                            u1, v2, u2,
                            u1, u2, u3,
                            u3, u2, v3 });
        }

        triangles = std::move(nextTriangles);
    }

    generateFromTriangles(triangles);

    std::uniform_real_distribution<float> distribution(0.0, 1.0);

    int n = vertices.size();
    for (int i = 0; i < n; i++)
        colors.push_back(distribution(rng));
}