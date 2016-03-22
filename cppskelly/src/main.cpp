#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <chrono>

#define GLM_FORCE_RADIANS 1
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL.h>
#include <SDL_image.h>
 
#include "glad/glad.h"
#include "logUtils.h"
#include "Shader.h"


static const int SCREEN_FULLSCREEN = 0;
static const int SCREEN_WIDTH  = 960;
static const int SCREEN_HEIGHT = 540;
static SDL_Window *window = nullptr;
static SDL_GLContext maincontext;
 
//// Our object has 4 points
//const GLuint points = 12;
//
//// Each poin has three values ( x, y, z)
//const GLuint floatsPerPoint = 3;
//
//// Each color has 4 values ( red, green, blue, alpha )
//const GLuint floatsPerColor = 4;
//
//// This is the object we'll draw ( a simple square
//const GLfloat diamond[points][floatsPerPoint] = {
//	{ 0.2f, 0.2f, 0.5f }, // Top right
//	{ -0.2f, 0.2f, 0.5f }, // Top left
//	{ 0.0f, 0.0f, 0.5f }, // Center
//
//	{ 0.2f, 0.2f, 0.5f }, // Top right
//	{ 0.2f, -0.2f, 0.5f }, // Bottom right 
//	{ 0.0f, 0.0f, 0.5f }, // Center
//
//	{ -0.2f, -0.2f, 0.5f }, // Bottom left
//	{ 0.2f, -0.2f, 0.5f }, // Bottom right 
//	{ 0.0f, 0.0f, 0.5f }, // Center
//
//	{ -0.2f, -0.2f, 0.5f }, // Bottom left
//	{ -0.2f, 0.2f, 0.5f }, // Top left
//	{ 0.0f, 0.0f, 0.5f }, // Center
//};
//
//// This is the object we'll draw ( a simple square
//const GLfloat colors[points][floatsPerColor] = {
//	{ 0.5f, 0.5f, 0.5f, 1.0f }, // Top right
//	{ 0.5f, 0.5f, 0.5f, 1.0f }, // Bottom right 
//	{ 0.0f, 0.0f, 0.0f, 1.0f }, // Center
//
//	{ 0.5f, 0.5f, 0.5f, 1.0f }, // Top left
//	{ 0.5f, 0.5f, 0.5f, 1.0f }, // Top right
//	{ 0.0f, 0.0f, 0.0f, 1.0f }, // Center
//
//	{ 0.5f, 0.5f, 0.5f, 1.0f }, // Bottom left
//	{ 0.5f, 0.5f, 0.5f, 1.0f }, // Bottom right 
//	{ 0.0f, 0.0f, 0.0f, 1.0f }, // Center
//
//	{ 0.5f, 0.5f, 0.5f, 1.0f }, // Bottom left
//	{ 0.5f, 0.5f, 0.5f, 1.0f }, // Top left
//	{ 0.0f, 0.0f, 0.0f, 1.0f }, // Center
//};

// Our object has 4 points
const uint32_t points = 6;

// Each poin has three values ( x, y, z)
const uint32_t floatsPerPoint = 3;

// Each color has 4 values ( red, green, blue, alpha )
const uint32_t floatsPerUV= 2;

// This is the object we'll draw ( a simple square
const GLfloat diamond[points][floatsPerPoint] = {
	{ -0.5, -0.5, 0.5 }, // Bottom left
	{ 0.5, 0.5, 0.5 }, // Top right
	{ -0.5, 0.5, 0.5 }, // Top left
	{ -0.5, -0.5, 0.5 }, // Bottom left
	{ 0.5, -0.5, 0.5 }, // Bottom right 
	{ 0.5, 0.5, 0.5 }, // Top right
};

// This is the object we'll draw ( a simple square
const GLfloat texCoords[points][floatsPerUV] = {
	{ 0.0f, 1.0f }, // Bottom left
	{ 1.0f, 0.0f }, // Top right
	{ 0.0f, 0.0f }, // Top left
	{ 0.0f, 1.0f }, // Bottom left
	{ 1.0f, 1.0f }, // Bottom right 
	{ 1.0f, 0.0f}, // Top right
};


// Create variables for storing the ID of our VAO and VBO
GLuint vbo[2], vao[1];

// The positons of the position and color data within the VAO 
// (It's important to keep the shader params bound to the attribute in the same order as the indexes)
const GLuint positionAttributeIndex = 0, texCoordAttributeIndex = 1;

static void APIENTRY openglCallbackFunction( GLenum source, GLenum type,
  GLuint id, GLenum severity, GLsizei length,
  const GLchar* message, const void* userParam )
{
  (void)source; (void)type; (void)id; 
  (void)severity; (void)length; (void)userParam;
  logInfo(message);
  if (severity == GL_DEBUG_SEVERITY_HIGH) 
  {
	  logError("Aborting...");
	  abort();
  }
}


bool init(const char * caption, bool fullscreen, int windowWidth, int windowHeight, int x = SDL_WINDOWPOS_CENTERED, int y = SDL_WINDOWPOS_CENTERED) 
{
  // Initialize SDL 
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		logError("Error initializing SDL");
		return false;
	}

	SDL_GL_LoadLibrary(nullptr); // Default OpenGL

	// Request an OpenGL 4.5 context (should be core)
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	// Also request a depth buffer
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	// Request a debug context.
	SDL_GL_SetAttribute(
	SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG
	);

	// Create the window
	if (fullscreen) 
	{
		window = SDL_CreateWindow(caption,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL );
	} 
	else 
	{
		window = SDL_CreateWindow(caption, x, y, windowWidth, windowHeight, SDL_WINDOW_OPENGL);
	}

	if (window == nullptr)
	{
		logError("Window couldn't be created!");
		std::string error = SDL_GetError();

		if (error != "")
		{
			logError(error.c_str());
			SDL_ClearError();
		}
		return false;
	}

	maincontext = SDL_GL_CreateContext(window);
	if (maincontext == NULL)
	{
		logError("Failed to create OpenGL context");
		return false;
	}

	// Check OpenGL properties
	logInfo("OpenGL loaded");
	gladLoadGLLoader(SDL_GL_GetProcAddress);

	std::ostringstream log("");
	log << "Vendor: " << glGetString(GL_VENDOR);
	logInfo(log.str().c_str());
	log.str("");
	log.clear();
	log << "Renderer: " << glGetString(GL_RENDERER);
	logInfo(log.str().c_str());
	log.str("");
	log.clear();
	log << "Version: " << glGetString(GL_VERSION);
	logInfo(log.str().c_str());


	// Enable the debug callback
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openglCallbackFunction, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);

	// Use v-sync
	SDL_GL_SetSwapInterval(1);

	// Disable depth test and face culling.
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	int w,h;
	SDL_GetWindowSize(window, &w, &h);
	glViewport(0, 0, w, h);
	glClearColor(0.0f, 0.5f, 1.0f, 0.0f);

	return true;
}

void close(SDL_Window* window, SDL_GLContext context, Shader* s)
{
	// Cleanup all the things we bound and allocated
	s->cleanUp();

	glDisableVertexAttribArray(0);
	glDeleteBuffers(1, vbo);
	glDeleteVertexArrays(1, vao);

	// Delete our OpengL context
	SDL_GL_DeleteContext(context);

	// Destroy our window
	SDL_DestroyWindow(window);

	// Shutdown SDL 2
	SDL_Quit();
}


void initGeometry()
{
	glCreateBuffers(2, vbo);
	glCreateVertexArrays(1, vao);
	glBindVertexArray(vao[0]); // current vtex array

	// pos
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, (points * floatsPerPoint) * sizeof(GLfloat), diamond, GL_STATIC_DRAW);

	glVertexAttribPointer(positionAttributeIndex, floatsPerPoint, GL_FLOAT, GL_FALSE, 0, 0); // Coord. info => Atr. index #0, three floats/vtx

	// UVS
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, (points * floatsPerUV) * sizeof(GLfloat), texCoords, GL_STATIC_DRAW);

	glVertexAttribPointer(texCoordAttributeIndex, floatsPerUV, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

float switchTimeout;
bool drawLine;
void update(float dt)
{

}

void render(SDL_Window* window, Shader* s, GLuint texID, GLuint samplerID)
{
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	s->bindAttributeLocation(positionAttributeIndex, "inPos");
	s->bindAttributeLocation(texCoordAttributeIndex, "inTexCoord");

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, texID);
	glBindSampler(0, samplerID);
	s->registerUniform1i("texture",0);
	s->useProgram();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(vao[0]);
	glEnableVertexAttribArray(positionAttributeIndex);
	glEnableVertexAttribArray(texCoordAttributeIndex);
	
	glDrawArrays(GL_TRIANGLES, 0, points);

	SDL_GL_SwapWindow(window);
}

void handleEvents(SDL_Event& event, bool& quit)
{
	while (SDL_PollEvent(&event)) 
	{
		if (event.type == SDL_QUIT) 
		{
			quit = true;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
			{
				quit = true;
				break;
			}
			default:break;
			}
		}
	}
}

bool loadTexture(const std::string& fileName, GLuint& texture, GLuint& width, GLuint& height)
{
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
	}
	else {
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

int main(int argc, char* args[])
{
	if (!init("OpenGL 4.5", false, 800, 600))
	{
		return -1;
	}

	GLuint texture;
	GLuint width, height;
	loadTexture("data/textures/chara_b.png", texture, width, height);

	initGeometry();

	GLuint samplerID;
	glCreateSamplers(1, &samplerID);

	Shader s;

	/*const int numShaders = 3;
	const char* fileNames[numShaders] = { "data/shader/test.vert", "data/shader/test.geom", "data/shader/test.frag" };*/
	const int numShaders = 2;
	const char* fileNames[numShaders] = { "data/shader/text.vert", "data/shader/text.frag" };
	GLenum shaderTypes[numShaders] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
	if (!s.init(fileNames, shaderTypes, numShaders))
	{
		logError("Shader init failed");
		int i;
		std::cin >> i;
		return 1;
	}
  
	SDL_Event event;
	bool quit = false;
	
	switchTimeout = 0.0f;
	drawLine = true;
	// Init timer!
	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();
	float elapsedSecs = 0.0f;
	while (!quit) 
	{    
		end = std::chrono::system_clock::now();
		float elapsedSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(end - start).count();
		start = end;
		handleEvents(event, quit);
		update(elapsedSeconds);
		render(window, &s, texture, samplerID);
	}

	close(window, maincontext, &s);
	return 0;
}


