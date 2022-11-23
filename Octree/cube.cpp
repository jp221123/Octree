#include "cube.h"

const Box& Cube::boundary(){
	if (!isDirty)
		return boundaryCache;
	isDirty = false;
	auto& c = center();
	auto h = halfside();
	return boundaryCache = Box(c[0] - h, c[1] - h, c[2] - h, c[0] + h, c[1] + h, c[2] + h);
}
