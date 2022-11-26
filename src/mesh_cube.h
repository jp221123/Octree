#pragma once
#include "mesh.h"

class CubeMesh: public Mesh {
private:
    void generateMesh(std::mt19937& rng) override final;
public:
    CubeMesh() : Mesh() {}
    CubeMesh(std::mt19937& rng);
    CubeMesh(const CubeMesh& other) : Mesh(other) {}
    CubeMesh(CubeMesh&& other) noexcept : Mesh(std::move(other)) {}
    CubeMesh& operator=(const CubeMesh& other){ Mesh::operator=(other);  return *this; }
    CubeMesh& operator=(CubeMesh&& other) noexcept { Mesh::operator=(std::move(other)); return *this; };
};