R"(
#version 330 core

// phong shading: brought from http://www.cs.toronto.edu/~jacobson/phong-demo/

// Input vertex data, different for all executions of this shader.
// vertex information in the model space
layout(location = 0) in vec3 vertexPosition;
// layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;

uniform mat4 pMat, mvMat, normalMat;

out vec3 normalInterp;
out vec3 vertPos;

// Output data ; will be interpolated for each fragment.
// out vec3 fragmentColor;

void main(){
	vec4 vertPos4 = mvMat * vec4(vertexPosition, 1.0);
	vertPos = vec3(vertPos4) / vertPos4.w;
	normalInterp = vec3(normalMat * vec4(vertexNormal, 0.0));
	gl_Position = pMat * vertPos4;

	// The color of each vertex will be interpolated
	// to produce the color of each fragment
	// fragmentColor = vertexColor;
}
)"