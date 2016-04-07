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

#include <map>

//Define this somewhere in your header file
#define BUFFER_OFFSET(i) ((void*)(i))

enum class PivotType
{
	TopLeft,
	Top,
	TopRight,
	CentreLeft,
	Centre,
	CentreRight,
	BotLeft,
	Bottom,
	BotRight,
	Custom
};

struct Rect
{
	float x, y, w, h;

	Rect() : x(0.0f), y(0.0f), w(0.0f), h(0.0f){}

	inline bool empty()
	{
		return w < FLT_EPSILON || h < FLT_EPSILON;
	}
};


struct Texture
{
	std::string path;
	unsigned int texID;
	unsigned int width;
	unsigned int height;
	int bpp;
	GLenum texFormat;
};


typedef std::map<std::string, Texture> TTextureTable;
typedef TTextureTable::iterator TTextureTableIter;
typedef std::map<std::string, Shader> TShaderTable;
typedef std::map<std::string, Shader>::iterator TShaderTableIter;

TTextureTable gTextures;
TShaderTable gShaders;

bool loadTexture(const std::string& fileName, GLuint& texture, GLuint& width, GLuint& height)
{
	TTextureTableIter value = gTextures.find(fileName);
	if (value != gTextures.end())
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
		gTextures[t.path] = t;
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

struct Sprite
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

};

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



void initSprite(Sprite* sprite, const std::string& texPath, const std::string& shaderName, float w, float h)
{
	sprite->texPath = texPath;
	sprite->shaderName = shaderName;
	unsigned int texWidth;
	unsigned int texHeight;
	loadTexture(sprite->texPath, sprite->texID, texWidth, texHeight);	

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
}

void cleanupSprite(Sprite* sprite)
{
	glDisableVertexAttribArray(0);
	glDeleteBuffers(NUM_SPRITE_VBO, sprite->vboIDs);
	glDeleteBuffers(1, &sprite->eboID);
	glDeleteVertexArrays(NUM_SPRITE_VAO, &sprite->vaoID);
	glDeleteSamplers(1, &sprite->samplerID);
}



struct OrthoCamera
{
	glm::vec3 eye;
	glm::vec3 up;
	glm::vec3 target;

	float left;
	float right;
	float top;
	float bot;
	float zNear;
	float zFar;
	
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
};

void initCamera(OrthoCamera* cam)
{
	cam->viewMatrix = glm::lookAt(cam->eye, cam->target, cam->up);
	cam->projMatrix = glm::ortho(cam->left, cam->right, cam->bot, cam->top, cam->zNear, cam->zFar);
}

//void rotateCamera(OrthoCamera* cam, const glm::vec3& axis, float angle)
//{
//	cam->viewMatrix = rotate(cam->viewMatrix, angle, axis);
//	cam->up = cam->up * glm::rotate(glm::mat4(1.0f), angle, axis);
//	cam->target = rotate(cam->target, angle, axis);
//}
//
//void translateCamera(OrthoCamera* cam, const glm::vec3& translationVector)
//{
//	cam->viewMatrix = translate(cam->viewMatrix, translationVector);
//	cam->eye = translate(cam->eye, translationVector);
//	cam->target = translate(cam->eye)
//}
//
//void zoomCamera(OrthoCamera* cam, float value)
//{
//	float newWidth = (cam->right - cam->left) / value;
//	cam->left = 
//}

void renderSprite(OrthoCamera* cam, Sprite* sprite)
{
	glm::mat4 mvp = cam->projMatrix * cam->viewMatrix * sprite->modelMatrix;
	// Pass matrices, setup shader params, etc
	TShaderTableIter shaderIt = gShaders.find(sprite->shaderName);
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
	glBindTexture(GL_TEXTURE_2D, sprite->texID);
	glBindSampler(TEX_UNIT, sprite->samplerID);

	shader.registerUniform1i("texture", 0);
	shader.useProgram();

	if (sprite->alphaBlend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	glBindVertexArray(sprite->vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, sprite->vboIDs[SPRITE_VBO_ATTR_POS]);
	glVertexAttribPointer(SPRITE_VBO_ATTR_POS, SPRITE_FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, 0); // Coord. info => Atr. index #0, three floats/vtx
	glEnableVertexAttribArray(SPRITE_VBO_ATTR_POS);
	glBindBuffer(GL_ARRAY_BUFFER, sprite->vboIDs[SPRITE_VBO_ATTR_UV]);
	glVertexAttribPointer(SPRITE_VBO_ATTR_UV, SPRITE_FLOATS_PER_UV, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(SPRITE_VBO_ATTR_UV);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprite->eboID);

	//glDrawArrays(GL_TRIANGLES, 0, NUM_SPRITE_TRIANGLES_VERT_COUNT);
	glDrawElements(GL_TRIANGLES, NUM_SPRITE_TRIANGLES_IDX_COUNT, GL_UNSIGNED_INT, nullptr);	
	//glDrawArrays(GL_TRIANGLES, 0, NUM_SPRITE_TRIANGLES_VERT_COUNT);
}

static const int SCREEN_FULLSCREEN = 0;
static const int SCREEN_WIDTH  = 960;
static const int SCREEN_HEIGHT = 540;
static SDL_Window *window = nullptr;
static SDL_GLContext maincontext;
static OrthoCamera cam;


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

void close(SDL_Window* window, SDL_GLContext context, Sprite* sprite)
{
	// Cleanup all the things we bound and allocated
	for (TShaderTableIter it = gShaders.begin(); it != gShaders.end(); ++it)
	{
		it->second.cleanUp();
	}

	cleanupSprite(sprite);

	// Delete our OpengL context
	SDL_GL_DeleteContext(context);

	// Destroy our window
	SDL_DestroyWindow(window);

	// Shutdown SDL 2
	SDL_Quit();
}


float switchTimeout;
bool drawLine;
void update(float dt)
{

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

void render(SDL_Window* window, OrthoCamera* c, Sprite* s)
{
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	renderSprite(c, s);

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

int main(int argc, char* args[])
{
	if (!init("OpenGL 4.5", false, 800, 600))
	{
		return -1;
	}

	cam.eye = { 0.0f, 0.0f, 10.0f };
	cam.target = { 0.0f, 0.0f, 0.0f };
	cam.up = { 0.0f, 1.0f, 0.0f };
	cam.left = -SCREEN_WIDTH/2;
	cam.right = SCREEN_WIDTH/2;
	cam.top = SCREEN_HEIGHT/2;
	cam.bot = -SCREEN_HEIGHT/2;
	cam.zNear = 1.0f;
	cam.zFar = -1.0f;
	initCamera(&cam);

	const int DEFAULT_SPRITE_SHADER_NUM_FILES = 2;
	const char* fileNames[DEFAULT_SPRITE_SHADER_NUM_FILES] = { "data/shader/text.vert", "data/shader/text.frag" };
	GLenum types[DEFAULT_SPRITE_SHADER_NUM_FILES] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
	createShader(DEFAULT_SHADER_NAME, fileNames, types, DEFAULT_SPRITE_SHADER_NUM_FILES);
	
	Sprite sprite("chara");
	initSprite(&sprite, "data/textures/chara_b.png", DEFAULT_SHADER_NAME, 0.2f, 0.2f);
	sprite.pos.x = sprite.pos.y = 0.0f;
	sprite.alphaBlend = true;
	glm::vec2 zero;
	setPivotType(&sprite, PivotType::Bottom);
	
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
		render(window, &cam, &sprite);
	}

	close(window, maincontext, &sprite);
	return 0;
}


