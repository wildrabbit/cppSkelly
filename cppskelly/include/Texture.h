#ifndef TEXTUREH_H
#define TEXTUREH_H

#include <string>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "GeomUtils.h"
#include "glad/glad.h"


typedef std::map<std::string, Texture> TTextureTable;
typedef TTextureTable::iterator TTextureTableIter;

class Texture
{

private:
		std::string path;
		GLuint texID;
		GLuint width;
		GLuint height;
		GLint bpp;
		GLenum texFormat;
	};
};
#endif