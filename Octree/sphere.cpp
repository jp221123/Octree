#include "sphere.h"

bool Sphere::intersects(Sphere* other, const float MARGIN) {
	glm::vec3 diff;
	for (int i = 0; i < 3; i++)
		diff[i] = center()[i] - other->center()[i];
	float dist = glm::length(diff);
	return dist < radius() + other->radius() + MARGIN;
}

std::ostream& operator<<(std::ostream& os, const Sphere& sphere){
	os << sphere.radius() << " (" << sphere.center()[0] << ',' << sphere.center()[1] << ',' << sphere.center()[2] << ')';
	return os;
}