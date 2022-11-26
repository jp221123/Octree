#include "cube.h"

const Box& Cube::boundary(){
	if (!isDirty)
		return boundaryCache;
	isDirty = false;
	auto& c = center();
	auto h = halfside();
	return boundaryCache = Box(c[0] - h, c[1] - h, c[2] - h, c[0] + h, c[1] + h, c[2] + h);
}

std::ostream& operator<<(std::ostream& os, const Cube& cube) {
	os << cube.halfside() << " (" << cube.center()[0] << ',' << cube.center()[1] << ',' << cube.center()[2] << ')' << std::endl;
	auto& c = cube.center();
	auto h = cube.halfside();
	auto boundary = Box(c[0] - h, c[1] - h, c[2] - h, c[0] + h, c[1] + h, c[2] + h);
	for(int i=0; i<3; i++)
		os << c[i] - h << ' ' << c[i] + h << std::endl;
	return os;
}