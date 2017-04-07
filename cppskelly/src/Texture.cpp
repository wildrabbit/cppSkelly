#include "Texture.h"
#include "logUtils.h"
#include <SDL_surface.h>
#include <SDL_image.h>
#include <sstream>

TTextureTable gTextures;


bool loadTexture(const std::string& fileName, GLuint& texture, GLuint& width, GLuint& height, TTextureTable& table)
{
	TTextureTableIter value = table.find(fileName);
	if (value != table.end())
	{
		logInfo("Texture was already loaded!");
		width = value->second.width;
		height = value->second.height;
		texture = value->second.texID;
		return true;
	}

	SDL_Surface *surface = IMG_Load(fileName.c_str()); // this surface will tell us the details of the image

	GLint nColors;
	GLenum textureFormat;

	if (surface)
	{
		// Check that the image’s width is a power of 2
		if ((surface->w & (surface->w - 1)) != 0)
		{
			logInfo("Warning: image.bmp’s width is not a power of 2");
		}

		// Also check if the height is a power of 2
		if ((surface->h & (surface->h - 1)) != 0)
		{
			logInfo("Warning: image.bmp’s height is not a power of 2");
		}

		//get number of channels in the SDL surface
		nColors = surface->format->BytesPerPixel;

		//contains an alpha channel
		if (nColors == 4)
		{
			if (surface->format->Rmask == 0x000000ff)
				textureFormat = GL_RGBA;
			else
				textureFormat = GL_BGRA;
		}
		else if (nColors == 3) //no alpha channel
		{
			if (surface->format->Rmask == 0x000000ff)
				textureFormat = GL_RGB;
			else
				textureFormat = GL_BGR;
		}
		else
		{
			logInfo("warning: the image is not truecolor...this will break ");
		}


		glCreateTextures(GL_TEXTURE_2D, 1, &texture);
		glTextureStorage2D(texture, 1, GL_RGBA8, surface->w, surface->h);
		glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureSubImage2D(texture, 0, 0, 0, surface->w, surface->h, textureFormat, GL_UNSIGNED_BYTE, surface->pixels);

		width = surface->w;
		height = surface->h;

		Texture t;
		t.path = fileName;
		t.width = surface->w;
		t.height = surface->h;
		t.bpp = nColors;
		t.texID = texture;
		t.texFormat = textureFormat;
		table[t.path] = t;
	}
	else
	{
		std::ostringstream sstream;
		sstream << "LoadTexture:: Could not load " << fileName.c_str() << ": " << SDL_GetError();
		logError(sstream.str().c_str());

		return false;
	}

	// Free the SDL_Surface only if it was successfully created
	if (surface) {
		SDL_FreeSurface(surface);
	}

	return true;
}

void Texture::cleanUp()
{
	glDeleteTextures(1, &texID);
}