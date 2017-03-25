#include <fstream>
#include <sstream>
#include <iostream>
#include "Shader.h"
#include "logUtils.h"

TShaderTable gShaders;

Shader::Shader()
{}

Shader::~Shader()
{}

std::string Shader::getFileContents(const std::string& file)
{
	std::ifstream t(file);
	std::stringstream buffer;
	buffer << t.rdbuf();

	// Make a std::string and fill it with the contents of buffer
	std::string fileContent = buffer.str();
	return fileContent;
}

void Shader::bindAttributeLocation(GLuint index, const std::string &attribute)
{
	glBindAttribLocation(mProgram, index, attribute.c_str());
}

void Shader::useProgram()
{
	glUseProgram(mProgram);
}

bool Shader::init(const char** filenames, GLenum* types, int numShaders)
{ 
	mShaderIds.clear();
	int result = 0;

	// Create and compile shaders
	for (int i = 0; i < numShaders; ++i)
	{
		GLuint shaderID = glCreateShader(types[i]);
		std::string fileContents = getFileContents(filenames[i]);
		const char* contents = fileContents.c_str();
		GLint size = (GLint)fileContents.length();
		glShaderSource(shaderID, 1, &contents, &size);
		glCompileShader(shaderID);

		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);

		if (result == 0)
		{
			traceShaderCompileError(shaderID);
			return false;
		}

		if (types[i] == GL_VERTEX_SHADER)
		{
			mVert = shaderID;
		}
		else if (types[i] == GL_FRAGMENT_SHADER)
		{
			mFrag = shaderID;
		}
		mShaderIds.push_back(shaderID);
	}

	mProgram = glCreateProgram();
	for (GLuint shaderID : mShaderIds)
	{
		glAttachShader(mProgram, shaderID);
	}
	
	// Link shaders
	glLinkProgram(mProgram);
	glGetProgramiv(mProgram, GL_LINK_STATUS, (int *)&result);
	if (result == 0)
	{
		traceShaderLinkError(mProgram);
		return false;
	}		

	for (GLuint shaderID : mShaderIds)
	{
		glDeleteShader(shaderID);
	}

	return true; 
}

void Shader::cleanUp()
{
	/* Cleanup all the things we bound and allocated */
	glUseProgram(0);

	for (GLuint shaderID : mShaderIds)
	{
		glDetachShader(mProgram, shaderID);
	}
	mShaderIds.clear();
	glDeleteProgram(mProgram);
}

void Shader::traceShaderLinkError(GLuint shaderId)
{
	std::ostringstream sstream("");
	sstream << "=======================================\n";
	sstream << "Shader linking failed : " << std::endl;

	// Find length of shader info log
	int maxLength;
	glGetProgramiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

	sstream << "Info Length : " << maxLength << std::endl;

	// Get shader info log
	char* shaderProgramInfoLog = new char[maxLength];
	glGetProgramInfoLog(mProgram , maxLength, &maxLength, shaderProgramInfoLog);

	sstream << "Linker error message : " << shaderProgramInfoLog << std::endl;

	logError(sstream.str().c_str());
	/* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
	/* In this simple program, we'll just leave */
	delete shaderProgramInfoLog;
	return;
}
void Shader::traceShaderCompileError(GLuint shaderId)
{
	std::ostringstream sstream("");
	sstream << "=======================================\n";
	sstream << "Shader compilation failed : " << std::endl;

	// Find length of shader info log
	int maxLength;
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

	// Get shader info log
	char* shaderInfoLog = new char[maxLength];
	glGetShaderInfoLog(shaderId, maxLength, &maxLength, shaderInfoLog);

	// Print shader info log
	sstream << "\tError info : " << shaderInfoLog << std::endl;
	sstream << "=======================================\n\n";

	logError(sstream.str().c_str());
	delete shaderInfoLog;
}

void Shader::registerUniform1i(const std::string& name,GLint value)
{
	GLint loc = glGetUniformLocation(mProgram, name.c_str());
	glProgramUniform1i(mProgram, loc, value);
}

void Shader::registerUniform1f(const std::string& name, GLfloat value)
{
	GLint loc = glGetUniformLocation(mProgram, name.c_str());
	glProgramUniform1f(mProgram, loc, value);
}

void Shader::registerUniformMatrix4f(const std::string& name, GLfloat* matrix)
{
	GLint loc = glGetUniformLocation(mProgram, name.c_str());
	glProgramUniformMatrix4fv(mProgram, loc, 1, GL_FALSE, matrix);
}