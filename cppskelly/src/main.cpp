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
  //logInfo(message);
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

class Tentacle
{
public:
	Tentacle(int idx, const glm::vec2& a, const glm::vec2& b, float speed, float t1, float t2, float amp, float width, unsigned int colour, float sideSpeed, float sideAmplitude)
		:a(a),
		b(b),
		controlSpeed(speed),
		segmentRatio1(t1),
		segmentRatio2(t2),
		controlAmplitude(amp),
		line("tentacle_" + std::to_string(idx)),
		time(0.f),
		width(width),
		colour(colour),
		numSteps(20),
		sideSpeed(sideSpeed),
		sideAmplitude(sideAmplitude)
	{}
	~Tentacle()
	{}

	void init()
	{
		line.colourRGBA[0] = ((colour & (0xff << 24)) >> 24)/(float)255.f;
		line.colourRGBA[1] = ((colour & (0xff << 16)) >> 16)/ (float)255.f;
		line.colourRGBA[2] = ((colour & (0xff << 8)) >> 8)/ (float)255.f;
		line.colourRGBA[3] = (colour & (0xff)) / (float)255.f;
		line.alphaBlend = true;
		int numPoints = numSteps + 1;

		std::vector<GLfloat> colours;
		float alphaStep = 1 / (float)numPoints;
		for (int i = 0; i < numPoints; ++i)
		{
			float red = ((colour & (0xff << 24)) >> 24) / (float)255.f;
			red *= (1 - alphaStep*i);
			colours.push_back(red);
			colours.push_back(((colour & (0xff << 16)) >> 16) / (float)255.f);
			colours.push_back(((colour & (0xff << 8)) >> 8) / (float)255.f);
			float alpha = (colour & (0xff)) / (float)255.f;
			alpha *= (1 - alphaStep*i);
			colours.push_back(alpha);
		}
		line.setPointColours(colours);

		line.linePointWidths.resize(numPoints);
		float step = 1 / (float)numPoints;
		for (int i = 0; i < numPoints; ++i)
		{
			line.linePointWidths[i] = width * (1 - step*i);
		}
		//line.lineWidth = width;
		line.shaderName = LINE_SHADER_NAME;

		time = 0.15f;

		glm::vec2 ab = b - a;
		float abLen = glm::length(ab);
		glm::vec2 dir = glm::normalize(ab);
		normal = glm::normalize(glm::vec2(-ab.y, ab.x ));

		ref1 = a + dir*(segmentRatio1*abLen);
		ref2 = a + dir*(segmentRatio2*abLen);

		updateControlPoints();
		initGeometry(line);
		updateGeometry(line);
	}

	void updateControlPoints()
	{
		glm::vec2 newb = b;
		newb.y += sideAmplitude * sin(sideSpeed * time);

		glm::vec2 ab = newb - a;
		float abLen = glm::length(ab);
		glm::vec2 dir = glm::normalize(ab);
		glm::vec2 localNormal = glm::normalize(glm::vec2(-ab.y, ab.x));

		ref1 = a + dir*(segmentRatio1*abLen);
		ref2 = a + dir*(segmentRatio2*abLen);

		float delta = cos(controlSpeed*time);		
		control1 = ref1 + localNormal*controlAmplitude  *delta;
		control2 = ref2 - localNormal*controlAmplitude *delta;
		cubicBezier(a, newb, control1, control2, line, numSteps);
	}

	void update(float dt)
	{
		time += dt;
		updateControlPoints();
		updateGeometry(line);
	}
	LineRenderer* getLine() 
	{
		return &line;
	}
private:
	float sideSpeed;
	float controlSpeed;
	float sideAmplitude;
	float segmentRatio1; // Percentage of ab, for the first control point
	float segmentRatio2; // Percentage of ab for the second...
	float controlAmplitude;
	float time;
	unsigned int colour;
	float width;
	int numSteps;

	glm::vec2 a;
	glm::vec2 b;
	glm::vec2 normal;
	glm::vec2 ref1;
	glm::vec2 ref2;

	glm::vec2 control1;
	glm::vec2 control2;
	LineRenderer line;
};

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

int main(int argc, char* args[])
{
	const int WINDOWS_WIDTH = 800;
	const int WINDOWS_HEIGHT = 600;
	if (!init("OpenGL 4.5", false, WINDOWS_WIDTH, WINDOWS_HEIGHT))
	{
		return -1;
	}

	const glm::vec3 CAM_EYE = { 0.0f, 0.0f, 0.8f };
	const glm::vec3 CAM_TARGET = glm::zero<glm::vec3>();
	const glm::vec3 CAM_UP = { 0.0f, 1.0f, 0.f };

	initOrtho(&gCam, CAM_EYE , CAM_TARGET, CAM_UP, { -SCREEN_WIDTH / 2, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, -SCREEN_HEIGHT / 2 }, -1.f, 1.f);
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
	initSprite(sprite, texPath, DEFAULT_SHADER_NAME, 64.f,64.f);
	sprite.pos.x = 2.0f;
	sprite.pos.y = 1.f;
	sprite.angle = 0.0f;
	sprite.scale.xy = 1.f;
	sprite.velocity.xy = 0.0f;
	sprite.speed = 300.0f;

	sprite.alphaBlend = true;
	setPivotType(sprite, PivotType::Centre);

	std::vector<Tentacle> tentacles;
	std::vector<Drawable*> tentacleViews;
	const float speed = 5.f;
	const float maxLineWidth = 8.f;
	const unsigned int colour = 0x880fbbff;
	const float t1 = 0.25f;
	const float t2 = 0.75f;
	float amplitude = 200.f;
	const float tentacleLen = 450.f;

	glm::vec2 a = { 0.f,-270.f };
	float spread = 170.f;
	const int numTentacles = 16;
	tentacles.reserve(numTentacles);
	float spreadStep = spread / (float)numTentacles;

	float sideAmplitude = 25.f;
	float sideSpeed = 5.5f;

	int halvedTentacles = numTentacles / 2;
	for (int i = 0; i < halvedTentacles; ++i)
	{
		glm::vec2 b = { a.x + tentacleLen * cos(glm::radians(spread)), a.y + tentacleLen * sin(glm::radians(spread)) };
		int sign = sgn(b.x);
		tentacles.emplace_back(2*i + 1, a, b, speed * sign, t1, t2, amplitude * sign, maxLineWidth, colour, sideSpeed, sideAmplitude);
		tentacles.back().init();
		tentacleViews.push_back(tentacles.back().getLine());

		b.x = -b.x;
		tentacles.emplace_back(2*(i+ 1), a, b, speed * sign, t1, t2, amplitude * -sign, maxLineWidth, colour, sideSpeed, sideAmplitude);
		tentacles.back().init();
		tentacleViews.push_back(tentacles.back().getLine());

		amplitude *= 0.9f;
		sideAmplitude *= 0.9f;
		spread -= spreadStep;
	}

	//Tentacle t(0, { 0.f,-300 }, { 400.f,0.f }, -3.f, 0.25f, 0.75f, 200.f, 6.f, 0xAA33EEFF);
	//t.init();
	//Tentacle t1(0, { 0.f,-300 }, { -400.f,0.f }, 3.f, 0.25f, 0.75f, -200.f, 6.f, 0xAA33EEFF);
	//t1.init();
	//Tentacle t2(0, { 0.f,-300 }, { 0.f,300.f }, -3.f, 0.25f, 0.75f, 200.f, 6.f, 0xAA33EEFF);
	//t2.init();
	//Tentacle t2b(0, { 0.f,-300 }, { 0.f,300.f }, 3.f, 0.25f, 0.75f, -200.f, 6.f, 0xAA33EEFF);
	//t2b.init();
	//Tentacle t3(0, { 0.f,-300 }, { -300.f,200.f }, 3.f, 0.25f, 0.75f, -200.f, 6.f, 0xAA33EEFF);
	//t3.init();
	//Tentacle t4(0, { 0.f,-300 }, { 300.f,200.f }, -3.f, 0.25f, 0.75f, 200.f, 6.f, 0xAA33EEFF);
	//t4.init();

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
		//update(elapsedSeconds, &input, &sprite);
		for (Tentacle& t : tentacles)
		{
			t.update(elapsedSeconds);
		}
		render(window, &gCam, tentacleViews);
	}

	close(window, maincontext, tentacleViews);
	return 0;
}


