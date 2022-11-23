#pragma once

#include "object.h"

#include <iostream>
#include <string>

class Sphere: public SolidBody {
public:
	using SolidBody::SolidBody;
	//Sphere(Mesh& mesh, std::mt19937& rng) : SolidBody(mesh, rng) {}

	const glm::vec3& center() const { return worldPos[stateIndex]; }
	float radius() const { return scaledFactor[stateIndex]; }

	bool intersects(Sphere*, const float MARGIN = 0.01);
};

std::ostream& operator<<(std::ostream& os, const Sphere& sphere);