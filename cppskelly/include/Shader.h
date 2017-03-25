#ifndef SHADERH_H
#define SHADERH_H

#include <string>
#include <glad\glad.h>
#include <vector>
#include <map>
class Shader
{
private:
	GLuint mProgram;
	GLuint mFrag, mVert;
	std::vector<GLuint> mShaderIds;

	void traceShaderLinkError(GLuint shaderId);
	void traceShaderCompileError(GLuint shaderId);

public:
	Shader();
	~Shader();

	std::string getFileContents(const std::string& file);

	void bindAttributeLocation(GLuint index, const std::string &attribute);
	void useProgram();

	bool init(const char** filenames, GLenum* types, int numShaders);
	void cleanUp();

	void registerUniform1i(const std::string& name,GLint value);
	void registerUniform1f(const std::string& name, GLfloat value);
	void registerUniformMatrix4f(const std::string& name, GLfloat* matrix);

	inline GLuint GetShaderID() const
	{
		return mProgram;
	}
};

typedef std::map<std::string, Shader> TShaderTable;
typedef std::map<std::string, Shader>::iterator TShaderTableIter;

extern TShaderTable gShaders;

#endif