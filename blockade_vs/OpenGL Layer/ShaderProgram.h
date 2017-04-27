#pragma once
#include "VoxelRenderBase.h"

class ShaderProgram
{
public:
	ShaderProgram();
	/*
	Pre Condition:
	-None
	Post Condition:
	-Initializes variables
	Side Effects:
	-None
	*/

	virtual ~ShaderProgram();
	/*
	Pre Condition:
	-None
	Post Condition:
	-Frees shader program
	Side Effects:
	-None
	*/


	virtual void freeProgram();
	/*
	Pre Condition:
	-None
	Post Condition:
	-Frees shader program if it exists
	Side Effects:
	-None
	*/

	bool bind();
	/*
	Pre Condition:
	-A loaded shader program
	Post Condition:
	-Sets this program as the current shader program
	-Reports to console if there was an error
	Side Effects:
	-None
	*/

	void unbind();
	/*
	Pre Condition:
	-None
	Post Condition:
	-Sets default shader program as current program
	Side Effects:
	-None
	*/

	GLuint getProgramID();
	/*
	Pre Condition:
	-None
	Post Condition:
	-Returns program ID
	Side Effects:
	-None
	*/

	void printProgramLog(GLuint program);
	/*
	Pre Condition:
	-None
	Post Condition:
	-Prints program log
	-Reports error is GLuint ID is not a shader program
	Side Effects:
	-None
	*/

	void printShaderLog(GLuint shader);
	/*
	Pre Condition:
	-None
	Post Condition:
	-Prints shader log
	-Reports error is GLuint ID is not a shader
	Side Effects:
	-None
	*/

	GLuint addShaderFromFile(std::string path, GLenum shaderType);
	/*
	Pre Condition:
	-None
	Post Condition:
	-Returns the ID of a compiled shader of the specified type from the specified file
	-Reports error to console if file could not be found or compiled
	Side Effects:
	-None
	*/
	int GetAttributeLocation(const std::string& attrib);

	int GetUniformLocation(const std::string& uni);
	GLuint linkProgram();
	//Program ID
	GLuint mProgramID;
	vector<GLuint> shaders;
};
