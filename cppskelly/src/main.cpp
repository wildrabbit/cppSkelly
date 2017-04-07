#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <chrono>

#define GLM_SWIZZLE 
#define GLM_FORCE_RADIANS 1

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <SDL.h>
#include <SDL_image.h>
#include <map>

#include "glad/glad.h"
#include "Drawable.h"
#include "GeomUtils.h"

#include "logUtils.h"
#include "Shader.h"
#include "Camera.h"
#include "Line.h"
#include "Sprite.h"
#include "Texture.h"

static const int SCREEN_FULLSCREEN = 0;
static const int SCREEN_WIDTH  = 800;
static const int SCREEN_HEIGHT = 600;
static SDL_Window *window = nullptr;
static SDL_GLContext maincontext;

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
	glClearColor(0.0f, 0.0f, 0.1f, 0.0f);

	return true;
}

void close(SDL_Window* window, SDL_GLContext context, std::vector<Drawable*> drawables)
{
	// Cleanup all the things we bound and allocated
	for (auto item : drawables)
	{
		item->cleanup();
	}

	for (TShaderTableIter it = gShaders.begin(); it != gShaders.end(); ++it)
	{
		it->second.cleanUp();
	}

	for (TTextureTableIter it = gTextures.begin(); it != gTextures.end(); ++it)
	{
		it->second.cleanUp();
	}



	// Delete our OpengL context
	SDL_GL_DeleteContext(context);

	// Destroy our window
	SDL_DestroyWindow(window);

	// Shutdown SDL 2
	SDL_Quit();
}

struct Input
{
	float xAxis;
	float yAxis;

	void reset()
	{
		xAxis = yAxis = 0.0f;
	}
};

float switchTimeout;
bool drawLine;

void update(float dt, Input* input, Sprite* sprite)
{
	sprite->velocity.x = input->xAxis * sprite->speed;
	sprite->velocity.y = input->yAxis * sprite->speed;

	sprite->pos.x += sprite->velocity.x * dt;
	sprite->pos.y += sprite->velocity.y * dt;
	
	updateMatrixSprite(sprite);
}


bool createShader(const std::string& name, const char* files[], GLenum* types, int numFiles)
{
	Shader s;
	if (!s.init(files, types, numFiles))
	{
		logError("Shader init failed");
		int i;
		std::cin >> i;
		return false;
	}
	gShaders[name] = s;
	return true;
}

void render(SDL_Window* w, Camera* c, const std::vector<Drawable*>& drawableObjects)
{
	glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (auto drawable : drawableObjects)
	{
		drawable->draw(w, c);
	}

	SDL_GL_SwapWindow(w);

}


void handleInput(SDL_Event& event, bool& quit, Input* input)
{
	input->reset();

	const Uint8* keyState = SDL_GetKeyboardState(NULL);

	if (keyState[SDL_SCANCODE_ESCAPE])
	{
		quit = true;
	}
	else
	{
		if (keyState[SDL_SCANCODE_LEFT])
		{
			input->xAxis = -1.0f;
		}
		else if (keyState[SDL_SCANCODE_RIGHT])
		{
			input->xAxis = 1.0f;
		}

		if (keyState[SDL_SCANCODE_UP])
		{
			input->yAxis = 1.0f;
		}
		else if (keyState[SDL_SCANCODE_DOWN])
		{
			input->yAxis = -1.0f;
		}
	}
	
		
	while (SDL_PollEvent(&event)) 
	{
		if (event.type == SDL_QUIT) 
		{
			quit = true;
		}
	}
}

int main(int argc, char* args[])
{
	const int WINDOWS_WIDTH = 800;
	const int WINDOWS_HEIGHT = 600;
	if (!init("OpenGL 4.5", false, WINDOWS_WIDTH, WINDOWS_HEIGHT))
	{
		return -1;
	}

	initOrtho(&gCam, { 0.0f, 0.0f, 0.8f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { -SCREEN_WIDTH / 2, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, -SCREEN_HEIGHT / 2 }, -1.f, 1.f);
	updateCameraViewMatrix(&gCam);
	updateCameraProjectionMatrix(&gCam);

	//initPerspective(&gPerspectiveCam, { 0.f, 0.f, 965.68f }, { 0.f, 0.f,0.f }, { 0.f, 1.f,0.f }, glm::radians(45.f), WINDOWS_WIDTH / (float)WINDOWS_HEIGHT, 0.1f, 965.68f);
	//updateCameraViewMatrix(&gPerspectiveCam);
	//updateCameraProjectionMatrix(&gPerspectiveCam);

	const int DEFAULT_SPRITE_SHADER_NUM_FILES = 2;
	const char* fileNames[DEFAULT_SPRITE_SHADER_NUM_FILES] = { "data/shader/text.vert", "data/shader/text.frag" };

	const int LINE_SHADER_NUM_FILES = 2;
	const char* lineNames[LINE_SHADER_NUM_FILES] = { "data/shader/line.vert","data/shader/line.frag" };

	GLenum types[DEFAULT_SPRITE_SHADER_NUM_FILES] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
	createShader(DEFAULT_SHADER_NAME, fileNames, types, DEFAULT_SPRITE_SHADER_NUM_FILES);
	createShader(LINE_SHADER_NAME, lineNames, types, LINE_SHADER_NUM_FILES);

	static const std::string spriteName("chara");
	static const std::string texPath("data/textures/chara_b.png");
	Sprite sprite(spriteName);
	initSprite(&sprite, texPath, DEFAULT_SHADER_NAME);
	sprite.pos.x = 2.0f;
	sprite.pos.y = 1.f;
	sprite.angle = 0.0f;
	sprite.scale.xy = 1.f;
	sprite.velocity.xy = 0.0f;
	sprite.speed = 300.0f;

	updateMatrixSprite(&sprite);
	sprite.alphaBlend = true;
	glm::vec2 zero;
	setPivotType(&sprite, PivotType::Centre);



	LineRenderer line("line");
	line.shaderName = LINE_SHADER_NAME;
	line.colourRGBA[0] = 1.f;
	line.colourRGBA[1] = 0.f;
	line.colourRGBA[2] = 0.f;
	line.colourRGBA[3] = 1.f;

	line.lineWidth = 15.f;
	line.alphaBlend = true;
	createSegment({ -400, 0 }, { 400,0 }, line);

	initGeometry(line);
	updateGeometry(line);
	line.modelMatrix = glm::translate(glm::vec3(line.pos.x, line.pos.y, 0.0f))
		* glm::rotate(line.angle, glm::vec3(0.0f, 0.0f, 1.0f))
		* glm::scale(glm::vec3(line.scale.x, line.scale.y, 1.0f));

	LineRenderer line2("line2");
	line2.shaderName = LINE_SHADER_NAME;
	line2.colourRGBA[0] = 1.f;
	line2.colourRGBA[1] = 0.f;
	line2.colourRGBA[2] = 1.f;
	line2.colourRGBA[3] = 1.f;
	line2.pos = glm::zero<glm::vec2>();
	line2.angle = 0.f;
	line2.lineWidth = 15.f;
	line2.alphaBlend = true;
	std::vector<glm::vec2> points = {
		{ 0.f, -300.f },{ -50.f,-200.f },{ 50.f,-100.f },{ -50.f,0.f },{ 50.f,100.f }, {-50.f,200.f}, {0.f, 300.f}
	};
	createPolyline(points,line2);

	initGeometry(line2);
	updateGeometry(line2);
	line2.modelMatrix = glm::translate(glm::vec3(line2.pos.x, line2.pos.y, 0.0f))
		* glm::rotate(line2.angle, glm::vec3(0.0f, 0.0f, 1.0f))
		* glm::scale(glm::vec3(line2.scale.x, line2.scale.y, 1.0f));


	LineRenderer line3("line3");
	line3.shaderName = LINE_SHADER_NAME;
	line3.colourRGBA[0] = 0.3f;
	line3.colourRGBA[1] = 0.8f;
	line3.colourRGBA[2] = 0.8f;
	line3.colourRGBA[3] = 1.f;
	line3.pos = glm::zero<glm::vec2>();
	line3.angle = 0.f;
	line3.lineWidth = 1.f;
	line3.alphaBlend = true;
	createQuadraticBezier({ -400.f, 30.f }, { 0.f, 30.f }, { -200.f, 220.f },line3, 10);

	initGeometry(line3);
	updateGeometry(line3);
	line3.modelMatrix = glm::translate(glm::vec3(line3.pos.x, line3.pos.y, 0.0f))
		* glm::rotate(line3.angle, glm::vec3(0.0f, 0.0f, 1.0f))
		* glm::scale(glm::vec3(line3.scale.x, line3.scale.y, 1.0f));

	LineRenderer line4("line4");
	line4.shaderName = LINE_SHADER_NAME;
	line4.colourRGBA[0] = 0.8f;
	line4.colourRGBA[1] = 0.8f;
	line4.colourRGBA[2] = 0.8f;
	line4.colourRGBA[3] = 1.f;
	line4.pos = glm::zero<glm::vec2>();
	line4.angle = 0.f;
	line4.lineWidth = 1.f;
	line4.alphaBlend = true;
	createCubicBezier({ 0, 30.f }, { 400.f, 30.f }, { 100, 220.f }, {300.f,-190.f}, line4, 20);

	initGeometry(line4);
	updateGeometry(line4);
	line4.modelMatrix = glm::translate(glm::vec3(line4.pos.x, line4.pos.y, 0.0f))
		* glm::rotate(line4.angle, glm::vec3(0.0f, 0.0f, 1.0f))
		* glm::scale(glm::vec3(line4.scale.x, line4.scale.y, 1.0f));

	SDL_Event event;
	bool quit = false;
	
	switchTimeout = 0.0f;
	drawLine = true;
	// Init timer!
	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();
	float elapsedSecs = 0.0f;
	Input input = { 0 };
	while (!quit) 
	{    
		end = std::chrono::system_clock::now();
		float elapsedSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(end - start).count();
		start = end;
		handleInput(event, quit, &input);
		update(elapsedSeconds, &input, &sprite);
		render(window, &gCam, { &line3, &sprite });
	}

	close(window, maincontext, { &line3, &sprite });
	return 0;
}


