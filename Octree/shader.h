#pragma once
#include <GL/glew.h>

#include <string>
class Shader {
public:
	GLuint programID{ 0 };
	virtual ~Shader() {
		if(programID != 0)
			glDeleteProgram(programID);
	}
protected:
	static GLuint loadShader(const std::string& VertexShaderCode, const std::string& FragmentShaderCode);
private:
	virtual void init() = 0;
};

class SolidBodyShader : public Shader {
public:
	GLuint pMatID, mvMatID, normalMatID;
	GLuint ambientColorID, diffuseColorID, specularColorID, shininessID;
	void init() override final;
};

class LineShader : public Shader {
public:
	GLuint vpMatID;
	GLuint lineColorID;
	void init() override final;
};