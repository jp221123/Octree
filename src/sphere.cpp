#include "sphere.h"

std::ostream& operator<<(std::ostream& os, const Sphere& sphere){
	os << sphere.radius() << " (" << sphere.center()[0] << ',' << sphere.center()[1] << ',' << sphere.center()[2] << ')';
	return os;
}