#ifndef TextureH_H
#define TextureH_H
#include <string>
#include <map>
#include <glad/glad.h>

struct Texture
{
	std::string path;
	unsigned int texID;
	unsigned int width;
	unsigned int height;
	int bpp;
	GLenum texFormat;

	void cleanUp();
};

typedef std::map<std::string, Texture> TTextureTable;
typedef TTextureTable::iterator TTextureTableIter;

bool loadTexture(const std::string& fileName, GLuint& texture, GLuint& width, GLuint& height, TTextureTable& table);

extern TTextureTable gTextures;


#endif
