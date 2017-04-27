#include "ShaderProgram.h"
#include <fstream>

ShaderProgram::ShaderProgram()
{
	mProgramID = NULL;
}

ShaderProgram::~ShaderProgram()
{
	//Free program if it exists
	freeProgram();
}

void ShaderProgram::freeProgram()
{
	//Delete program
	glDeleteProgram(mProgramID);
}

bool ShaderProgram::bind()
{
	//Use shader
	glUseProgram(mProgramID);

	//Check for error
//	GLenum error = glGetError();
//	if (error != GL_NO_ERROR)
//	{
//		printf("Error binding shader! error number: %d\n", error);
//		printProgramLog(mProgramID);
//		return false;
//	}

	return true;
}

void ShaderProgram::unbind()
{
	//Use default program
	glUseProgram(NULL);
}
int ShaderProgram::GetAttributeLocation(const std::string& attrib)
{
	return glGetAttribLocation(mProgramID, attrib.c_str());
}

int ShaderProgram::GetUniformLocation(const std::string& uni)
{
	return glGetUniformLocation(mProgramID, uni.c_str());
}
GLuint ShaderProgram::getProgramID()
{
	return mProgramID;
}

void ShaderProgram::printProgramLog(GLuint program)
{
	//Make sure name is shader
	if (glIsProgram(program))
	{
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log
		glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			//Print Log
			printf("%s\n", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf("Name %d is not a program\n", program);
	}
}

void ShaderProgram::printShaderLog(GLuint shader)
{
	//Make sure name is shader
	if (glIsShader(shader))
	{
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log
		glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			//Print Log
			printf("%s\n", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf("Name %d is not a shader\n", shader);
	}
}
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
GLuint ShaderProgram::addShaderFromFile(std::string path, GLenum shaderType)
{
	//Open file
	GLuint shaderID = 0;
	std::string shaderString;
	std::ifstream sourceFile(path.c_str());

	//Source file loaded
	if (sourceFile)
	{
		//Get shader source
		shaderString.assign((std::istreambuf_iterator< char >(sourceFile)), std::istreambuf_iterator< char >());

		//Create shader ID
		shaderID = glCreateShader(shaderType);

		//Set shader source
		const GLchar* shaderSource = shaderString.c_str();
		glShaderSource(shaderID, 1, (const GLchar**)&shaderSource, NULL);

		//Compile shader source
		glCompileShader(shaderID);

		//Check shader for errors
		GLint shaderCompiled = GL_FALSE;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);
		if (shaderCompiled != GL_TRUE)
		{
			printf("Unable to compile shader %d!\n\nSource:\n%s\n", shaderID, shaderSource);
			printShaderLog(shaderID);
			glDeleteShader(shaderID);
			shaderID = 0;
		}
		else {
			shaders.push_back(shaderID);
		}
	}
	else
	{
		printf("Unable to open file %s\n", path.c_str());
	}


	return shaderID;
}
GLuint ShaderProgram::linkProgram() {
	mProgramID = glCreateProgram();

	//Attach our shaders to our program
	for (auto it = shaders.begin(); it != shaders.end(); it++)
	{
		glAttachShader(mProgramID, *it);
	}
	//Link our program
	glLinkProgram(mProgramID);

	//Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(mProgramID, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(mProgramID, GL_INFO_LOG_LENGTH, &maxLength);

		//The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(mProgramID, maxLength, &maxLength, &infoLog[0]);
		printf("Unable to open file %s\n", &infoLog[0]);
		//We don't need the program anymore.
		glDeleteProgram(mProgramID);
		//Don't leak shaders either.
		for (auto it = shaders.begin(); it != shaders.end(); it++)
		{
			glDeleteShader(*it);
		}
		mProgramID = 0;
		return 0;
	}

	//Always detach shaders after a successful link.
	for (auto it = shaders.begin(); it != shaders.end(); it++)
	{
		glDetachShader(mProgramID, *it);
	}
	return mProgramID;
}