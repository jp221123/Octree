#pragma once

#include "object.h"
#include "sphere.h"

#include <iostream>
#include <string>

class Cube : public SolidBody {
public:
	// using SolidBody::SolidBody;
	Cube(Mesh& mesh, std::mt19937& rng) :
		SolidBody(mesh, rng, SolidBodyType::CUBE) {}

	Box boundaryCache;
	const Box& boundary();
	const glm::vec3& center() const { return worldPos[stateIndex]; }
	float halfside() const { return scaledFactor[stateIndex]; }
};