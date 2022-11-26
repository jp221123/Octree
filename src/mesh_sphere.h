#pragma once
#include "mesh.h"

class SphereMesh : public Mesh {
private:
    const int subdivision;
    void generateMesh(std::mt19937& rng) override final;
public:
    SphereMesh() : Mesh(), subdivision(0) {}
    SphereMesh(std::mt19937& rng, int subdivision);
    SphereMesh(const SphereMesh& other) = delete;
    SphereMesh(SphereMesh&& other) noexcept : Mesh(std::move(other)), subdivision(other.subdivision) {}
    SphereMesh& operator=(const SphereMesh& other) = delete;
    SphereMesh& operator=(SphereMesh&& other) noexcept = delete;
};