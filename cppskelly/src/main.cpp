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
#include "Texture.h"


//Define this somewhere in your header file
#define BUFFER_OFFSET(i) ((void*)(i))

TTextureTable gTextures;

// Forget about batching for now
const int NUM_SPRITE_VBO = 2;
const int NUM_SPRITE_VAO = 1;
const int NUM_SPRITE_TRIANGLES_VERT_COUNT = 4;
const int NUM_SPRITE_TRIANGLES_IDX_COUNT = 6;
const int SPRITE_VBO_ATTR_POS = 0;
const int SPRITE_VBO_ATTR_UV = 1;

const int SPRITE_FLOATS_PER_VERTEX = 2;
const int SPRITE_FLOATS_PER_UV = 2;

const char* DEFAULT_SHADER_NAME = "sprites_default";

struct Sprite: public Drawable
{
	std::string name;
	std::string texPath;
	std::string shaderName;
	Rect clipRect;
	unsigned int texID;
	unsigned int shaderID;
	unsigned int samplerID;
	glm::vec2 pos;
	glm::vec2 scale;
	float width;
	float height;
	float angle;

	float speed;

	glm::vec2 velocity;
	glm::vec2 acceleration;

	glm::vec2 pivot;
	glm::mat4 modelMatrix;

	bool alphaBlend;
	
	GLfloat vertices[NUM_SPRITE_TRIANGLES_VERT_COUNT][SPRITE_FLOATS_PER_VERTEX];
	GLfloat uvs[NUM_SPRITE_TRIANGLES_VERT_COUNT][SPRITE_FLOATS_PER_UV];

	unsigned int indexes[NUM_SPRITE_TRIANGLES_IDX_COUNT];
	unsigned int vaoID;
	unsigned int vboIDs[NUM_SPRITE_VBO];
	unsigned int eboID;

	Sprite(const std::string& name)
		:name(name)
		,texPath() ,shaderName()
		,texID(0), shaderID(0), samplerID(0)
		,clipRect()
		,width(0.0f), height(0.0f), angle(0.0f)
		,pos(), scale(1.0f, 1.0f)
		,pivot(), modelMatrix(), alphaBlend(false)
	{}

	void draw(SDL_Window* w, Camera* c) override;
	void cleanup() override;

};

void Sprite::draw(SDL_Window* w, Camera* cam)
{
	glm::mat4 mvp = cam->projMatrix * cam->viewMatrix * modelMatrix;
	// Pass matrices, setup shader params, etc
	TShaderTableIter shaderIt = gShaders.find(shaderName);
	if (shaderIt == gShaders.end())
	{
		logError("Shader not found!!");
		return;
	}

	Shader shader = shaderIt->second;
	shader.bindAttributeLocation(SPRITE_VBO_ATTR_POS, "inPos");
	shader.bindAttributeLocation(SPRITE_VBO_ATTR_UV, "inTexCoord");

	const int TEX_UNIT = 0;
	glActiveTexture(GL_TEXTURE0 + TEX_UNIT); // The 0 addition seems to be a convention to specify the texture unit
	glBindTexture(GL_TEXTURE_2D, texID);
	glBindSampler(TEX_UNIT, samplerID);

	shader.registerUniform1i("texture", 0);
	shader.registerUniformMatrix4f("mvp", (GLfloat*)glm::value_ptr(mvp));
	shader.useProgram();

	if (alphaBlend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	glBindVertexArray(vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[SPRITE_VBO_ATTR_POS]);
	glVertexAttribPointer(SPRITE_VBO_ATTR_POS, SPRITE_FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, 0); // Coord. info => Atr. index #0, three floats/vtx
	glEnableVertexAttribArray(SPRITE_VBO_ATTR_POS);
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[SPRITE_VBO_ATTR_UV]);
	glVertexAttribPointer(SPRITE_VBO_ATTR_UV, SPRITE_FLOATS_PER_UV, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(SPRITE_VBO_ATTR_UV);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);

	//glDrawArrays(GL_TRIANGLES, 0, NUM_SPRITE_TRIANGLES_VERT_COUNT);
	glDrawElements(GL_TRIANGLES, NUM_SPRITE_TRIANGLES_IDX_COUNT, GL_UNSIGNED_INT, nullptr);
	//glDrawArrays(GL_TRIANGLES, 0, NUM_SPRITE_TRIANGLES_VERT_COUNT);
}

void initGeometry(Sprite* sprite)
{
	sprite->vertices[0][0] = -sprite->pivot.x;
	sprite->vertices[0][1] = -sprite->pivot.y;
	sprite->vertices[1][0] = sprite->width - sprite->pivot.x;
	sprite->vertices[1][1] = sprite->height - sprite->pivot.y;
	sprite->vertices[2][0] = -sprite->pivot.x;
	sprite->vertices[2][1] = sprite->height - sprite->pivot.y;
	sprite->vertices[3][0] = sprite->width - sprite->pivot.x;
	sprite->vertices[3][1] = -sprite->pivot.y;

	sprite->indexes[0] = 0;
	sprite->indexes[1] = 1;
	sprite->indexes[2] = 2;
	sprite->indexes[3] = 0;
	sprite->indexes[4] = 3;
	sprite->indexes[5] = 1;

	sprite->uvs[0][0] = 0.0f;
	sprite->uvs[0][1] = 1.0f;
	sprite->uvs[1][0] = 1.0f;
	sprite->uvs[1][1] = 0.0f;
	sprite->uvs[2][0] = 0.0f;
	sprite->uvs[2][1] = 0.0f;
	sprite->uvs[3][0] = 1.0f;
	sprite->uvs[3][1] = 1.0f;

	glCreateVertexArrays(NUM_SPRITE_VAO, &(sprite->vaoID));
	glBindVertexArray(sprite->vaoID); // current vtex array

	glCreateBuffers(NUM_SPRITE_VBO, sprite->vboIDs);

	// pos
	glBindBuffer(GL_ARRAY_BUFFER, sprite->vboIDs[SPRITE_VBO_ATTR_POS]);
	glBufferData(GL_ARRAY_BUFFER, (NUM_SPRITE_TRIANGLES_VERT_COUNT * SPRITE_FLOATS_PER_VERTEX) * sizeof(GLfloat), sprite->vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(SPRITE_VBO_ATTR_POS, SPRITE_FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0)); // Coord. info => Atr. index #0, three floats/vtx
	glEnableVertexAttribArray(SPRITE_VBO_ATTR_POS);


	// UVS
	glBindBuffer(GL_ARRAY_BUFFER, sprite->vboIDs[SPRITE_VBO_ATTR_UV]);
	glBufferData(GL_ARRAY_BUFFER, (NUM_SPRITE_TRIANGLES_VERT_COUNT * SPRITE_FLOATS_PER_UV) * sizeof(GLfloat), sprite->uvs, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(SPRITE_VBO_ATTR_UV, SPRITE_FLOATS_PER_UV, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(SPRITE_VBO_ATTR_UV);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indexes
	glCreateBuffers(1, &sprite->eboID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprite->eboID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (NUM_SPRITE_TRIANGLES_IDX_COUNT)* sizeof(GLuint), sprite->indexes, GL_STATIC_DRAW);


}

void updateMatrixSprite(Sprite* sprite)
{
	sprite->modelMatrix = glm::translate(glm::vec3(sprite->pos.x, sprite->pos.y, 0.0f))
		* glm::rotate(sprite->angle, glm::vec3(0.0f, 0.0f, 1.0f))
		* glm::scale(glm::vec3(sprite->scale.x, sprite->scale.y, 1.0f));	
}

void updateGeometry(Sprite* sprite)
{
	sprite->vertices[0][0] = -sprite->pivot.x;
	sprite->vertices[0][1] = -sprite->pivot.y;
	sprite->vertices[1][0] = sprite->width - sprite->pivot.x;
	sprite->vertices[1][1] = sprite->height - sprite->pivot.y;
	sprite->vertices[2][0] = -sprite->pivot.x;
	sprite->vertices[2][1] = sprite->height - sprite->pivot.y;
	sprite->vertices[3][0] = sprite->width - sprite->pivot.x;
	sprite->vertices[3][1] = -sprite->pivot.y;

	glBindVertexArray(sprite->vaoID); // current vtex array
	// pos
	glBindBuffer(GL_ARRAY_BUFFER, sprite->vboIDs[SPRITE_VBO_ATTR_POS]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sprite->vertices), sprite->vertices);
	glVertexAttribPointer(SPRITE_VBO_ATTR_POS, SPRITE_FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, 0); // Coord. info => Atr. index #0, three floats/vtx


	if (!sprite->clipRect.empty())
	{
		Texture tex = gTextures[sprite->texPath];
		float clipX1Ratio = sprite->clipRect.x / tex.width;
		float clipY1Ratio = sprite->clipRect.y / tex.height;
		float clipX2Ratio = clipX1Ratio + sprite->clipRect.w / tex.width;
		float clipY2Ratio = clipY1Ratio + sprite->clipRect.h / tex.height;
		sprite->uvs[0][0] = clipX1Ratio;
		sprite->uvs[0][1] = clipY2Ratio;
		sprite->uvs[1][0] = clipX2Ratio;
		sprite->uvs[1][1] = clipY1Ratio;
		sprite->uvs[2][0] = clipX1Ratio;
		sprite->uvs[2][1] = clipY1Ratio;
		sprite->uvs[3][0] = clipX2Ratio;
		sprite->uvs[3][1] = clipY2Ratio;
	}
	else
	{
		sprite->uvs[0][0] = 0.0f;
		sprite->uvs[0][1] = 1.0f;
		sprite->uvs[1][0] = 1.0f;
		sprite->uvs[1][1] = 0.0f;
		sprite->uvs[2][0] = 0.0f;
		sprite->uvs[2][1] = 0.0f;
		sprite->uvs[3][0] = 1.0f;
		sprite->uvs[3][1] = 1.0f;
	}

	glBindBuffer(GL_ARRAY_BUFFER, sprite->vboIDs[SPRITE_VBO_ATTR_UV]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sprite->uvs), sprite->uvs);
	glVertexAttribPointer(SPRITE_VBO_ATTR_UV, SPRITE_FLOATS_PER_UV, GL_FLOAT, GL_FALSE, 0, 0); // Coord. info => Atr. index #0, three floats/vtx

	//Indexes will not change
}

void setCustomPivot(Sprite* sprite, glm::vec2* pivot, bool update = true)
{
	sprite->pivot.x = pivot->x;
	sprite->pivot.y = pivot->y;
	if (update)
		updateGeometry(sprite);
}

void setPivotType(Sprite* sprite, PivotType pivotType, bool update = true)
{
	glm::vec2 v = { 0.0f, 0.0f };
	switch (pivotType)
	{
	case PivotType::TopLeft:
	{
		v.y = sprite->height;
		break;
	}
	case PivotType::Top:
	{
		v.x = sprite->width * 0.5f;
		v.y = sprite->height;
		break;
	}
	case PivotType::TopRight:
	{
		v.x = sprite->width;
		v.y = sprite->height;
		break;
	}
	case PivotType::CentreLeft:
	{
		v.y = sprite->height * 0.5f;
		break;
	}
	case PivotType::Centre:
	{
		v.x = sprite->width * 0.5f;
		v.y = sprite->height * 0.5f;
		break;
	}
	case PivotType::CentreRight:
	{
		v.x = sprite->width;
		v.y = sprite->height * 0.5f;
		break;
	}
	case PivotType::BotLeft:
	{
		break;
	}
	case PivotType::Bottom:
	{
		v.x = sprite->width * 0.5f;
		break;
	}
	case PivotType::BotRight:
	{
		v.x = sprite->width;
		break;
	}
	default:
		break;
	}
	setCustomPivot(sprite, &v, update);
}
void initSprite(Sprite* sprite, const std::string& texPath, const std::string& shaderName)
{
	sprite->texPath = texPath;
	sprite->shaderName = shaderName;
	unsigned int texWidth;
	unsigned int texHeight;
	loadTexture(sprite->texPath, sprite->texID, texWidth, texHeight, gTextures);

	sprite->width = (float)texWidth;
	sprite->height = (float)texHeight;

	setPivotType(sprite, PivotType::Custom, false);

	initGeometry(sprite);

	TShaderTableIter it = gShaders.find(sprite->shaderName);
	if (it == gShaders.end())
	{
		logError("Shader not found!!");
	}
	else
	{
		sprite->shaderID = it->second.GetShaderID();
	}
	glCreateSamplers(1, &sprite->samplerID);

	updateMatrixSprite(sprite);
}

void initSprite(Sprite* sprite, const std::string& texPath, const std::string& shaderName, float w, float h)
{
	sprite->texPath = texPath;
	sprite->shaderName = shaderName;
	unsigned int texWidth;
	unsigned int texHeight;
	loadTexture(sprite->texPath, sprite->texID, texWidth, texHeight, gTextures);

	sprite->width = w;
	sprite->height = h;

	setPivotType(sprite, PivotType::Custom, false);

	initGeometry(sprite);

	TShaderTableIter it = gShaders.find(sprite->shaderName);
	if (it == gShaders.end())
	{
		logError("Shader not found!!");
	}
	else
	{
		sprite->shaderID = it->second.GetShaderID();
	}
	glCreateSamplers(1, &sprite->samplerID);

	updateMatrixSprite(sprite);
}

void Sprite::cleanup()
{
	glDisableVertexAttribArray(0);
	glDeleteBuffers(NUM_SPRITE_VBO, vboIDs);
	glDeleteBuffers(1, &eboID);
	glDeleteVertexArrays(NUM_SPRITE_VAO, &vaoID);
	glDeleteSamplers(1, &samplerID);
}

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
	glClearColor(0.5f, 0.5f, 0.0f, 0.0f);

	return true;
}

void close(SDL_Window* window, SDL_GLContext context, std::vector<Drawable*> drawables)
{
	// Cleanup all the things we bound and allocated
	for (TShaderTableIter it = gShaders.begin(); it != gShaders.end(); ++it)
	{
		it->second.cleanUp();
	}

	for (auto item : drawables)
	{
		item->cleanup();
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
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
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

	gCam.eye = { 0.0f, 0.0f, 0.8f };
	gCam.target = { 0.0f, 0.0f, 0.0f };
	gCam.up = { 0.0f, 1.0f, 0.0f };
	gCam.left = -SCREEN_WIDTH / 2;
	gCam.right = SCREEN_WIDTH / 2;
	gCam.top = SCREEN_HEIGHT / 2;
	gCam.bot = -SCREEN_HEIGHT / 2;
	gCam.zNear = 1.0f;
	gCam.zFar = -1.0f;
	updateCameraViewMatrix(&gCam);
	updateCameraProjectionMatrix(&gCam);

	//perCam.eye = { 0.f, 0.f, 965.68f }; // Still an approximation, the idea with this combination of zFar and eye.z was to get the sprite to render to the screen using pixel-perfect...ish coords
	//perCam.target = { 0.f, 0.f,0.f };
	//perCam.up = { 0.f, 1.f,0.f };
	//perCam.fov = glm::radians(45.f);
	//perCam.aspect = WINDOWS_WIDTH / (float)WINDOWS_HEIGHT;
	//perCam.zNear = 0.1f;
	//perCam.zFar = 965.68f;
	//updateCameraViewMatrix(&perCam);
	//updateCameraProjectionMatrix(&perCam);

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

	updateMatrixSprite(&sprite);
	sprite.alphaBlend = true;
	glm::vec2 zero;
	setPivotType(&sprite, PivotType::Centre);
	
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
		render(window, &gCam, { &sprite,&line, &line2, &line3,&line4 });
	}

	close(window, maincontext, { &line, &line2, &line3, &line4});
	return 0;
}


