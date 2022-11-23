#pragma once
#include "mesh.h"

class TriangleMesh: public Mesh {
private:
    void generateMesh(std::mt19937& rng) override final;
public:
    TriangleMesh() : Mesh() {}
    TriangleMesh(std::mt19937& rng);
    TriangleMesh(const TriangleMesh& other) : Mesh(other) {}
    TriangleMesh(TriangleMesh&& other) noexcept : Mesh(std::move(other)) {}
    TriangleMesh& operator=(const TriangleMesh& other) { Mesh::operator=(other);  return *this; }
    TriangleMesh& operator=(TriangleMesh&& other) noexcept { Mesh::operator=(std::move(other)); return *this; };
};