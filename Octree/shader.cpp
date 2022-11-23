#include "shader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

void SolidBodyShader::init(){
	const std::string vertexShaderCode =
		#include "vertex_shader.glsl"
	;
	const std::string fragmentShaderCode =
		#include "fragment_shader.glsl"
	;
	programID = loadShader(vertexShaderCode, fragmentShaderCode);
	ambientColorID = glGetUniformLocation(programID, "ambientColor");
	diffuseColorID = glGetUniformLocation(programID, "diffuseColor");
	mvMatID = glGetUniformLocation(programID, "mvMat");
	normalMatID = glGetUniformLocation(programID, "normalMat");
	pMatID = glGetUniformLocation(programID, "pMat");
	shininessID = glGetUniformLocation(programID, "shininess");
	specularColorID = glGetUniformLocation(programID, "specularColor");
}

void LineShader::init() {
	const std::string vertexShaderCode = R"(
		#version 330 core

		layout(location = 0) in vec3 vertexPosition;

		uniform mat4 vpMat;

		void main() {
			gl_Position = vpMat * vec4(vertexPosition, 1.0);
		}
	)";

	const std::string fragmentShaderCode = R"(
		#version 330 core

		uniform vec3 lineColor;

		out vec3 color;

		void main() {
			color = lineColor;
		}
	)";

	programID = loadShader(vertexShaderCode, fragmentShaderCode);
	vpMatID = glGetUniformLocation(programID, "vpMat");
	lineColorID = glGetUniformLocation(programID, "lineColor");
}

// brought from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
GLuint Shader::loadShader(const std::string& VertexShaderCode, const std::string& FragmentShaderCode) {
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	//std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	//if (VertexShaderStream.is_open()) {
	//	std::stringstream sstr;
	//	sstr << VertexShaderStream.rdbuf();
	//	VertexShaderCode = sstr.str();
	//	VertexShaderStream.close();
	//}
	//else {
	//	std::cout << "Impossible to open " << vertex_file_path << "\nAre you in the right directory ? Don't forget to read the FAQ !" << std::endl;
	//	return 0;
	//}

	// Read the Fragment Shader code from the file
	//std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	//if (FragmentShaderStream.is_open()) {
	//	std::stringstream sstr;
	//	sstr << FragmentShaderStream.rdbuf();
	//	FragmentShaderCode = sstr.str();
	//	FragmentShaderStream.close();
	//}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	// std::cout << "Compiling shader : " << vertex_file_path << std::endl;
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		std::cerr << &VertexShaderErrorMessage[0] << std::endl;
	}


	// Compile Fragment Shader
	// std::cout << "Compiling shader : " << fragment_file_path << std::endl;
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		std::cerr << &FragmentShaderErrorMessage[0] << std::endl;
	}

	// Link the program
	// printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		std::cerr << &ProgramErrorMessage[0] << std::endl;
	}
	std::cout << "Shader loaded" << std::endl;

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

