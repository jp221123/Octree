#pragma once

#include "object.h"
#include "cube.h"

#include <iostream>
#include <string>

class Sphere: public SolidBody {
public:
	// using SolidBody::SolidBody;
	Sphere(Mesh& mesh, std::mt19937& rng) : SolidBody(mesh, rng, SolidBodyType::SPHERE) {}

	const glm::vec3& center() const { return worldPos[stateIndex]; }
	float radius() const { return scaledFactor[stateIndex]; }
};

std::ostream& operator<<(std::ostream& os, const Sphere& sphere);