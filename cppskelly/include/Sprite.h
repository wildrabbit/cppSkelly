#ifndef SPRITEH_H
#define SPRITEH_H

#include "Drawable.h"
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "GeomUtils.h"
#include "glad/glad.h"

// Forget about batching for now
static const int NUM_SPRITE_VBO = 2;
extern const int NUM_SPRITE_VAO;
static const int NUM_SPRITE_TRIANGLES_VERT_COUNT = 4;
static const int NUM_SPRITE_TRIANGLES_IDX_COUNT = 6;
extern const int SPRITE_VBO_ATTR_POS;
extern const int SPRITE_VBO_ATTR_UV;

static const int SPRITE_FLOATS_PER_VERTEX = 2;
static const int SPRITE_FLOATS_PER_UV = 2;

extern const char* DEFAULT_SHADER_NAME;


struct Camera;
struct OrthoCamera;

struct Sprite: public Drawable
{
	//Internal repr.
	std::string name;
	std::string texPath;
	std::string shaderName;
		
	glm::vec2 pivot;

	//Render		
	unsigned int texID;
	unsigned int shaderID;
	unsigned int samplerID;
	glm::mat4 modelMatrix;

	GLfloat vertices[NUM_SPRITE_TRIANGLES_VERT_COUNT][SPRITE_FLOATS_PER_VERTEX];
	GLfloat uvs[NUM_SPRITE_TRIANGLES_VERT_COUNT][SPRITE_FLOATS_PER_UV];
	GLuint indexes[NUM_SPRITE_TRIANGLES_IDX_COUNT];
	GLuint vaoID;
	GLuint vboIDs[NUM_SPRITE_VBO];
	GLuint eboID;

	Rect clipRect;

	glm::vec2 pos;
	glm::vec2 scale;
	float width;
	float height;
	float angle;

	bool alphaBlend;

	Sprite(const std::string& name);
	virtual ~Sprite();

	void draw(SDL_Window* w, Camera* c) override;
	void cleanup() override;

	glm::vec2 velocity;
	glm::vec2 acceleration;
	float speed;
};

void initSprite(Sprite& sprite, const std::string& texPath, const std::string& shaderName);
void initSprite(Sprite& sprite, const std::string& texPath, const std::string& shaderName, float w, float h);
void setPivotType(Sprite& sprite, PivotType pivotType, bool update = true);
void setCustomPivot(Sprite& sprite, glm::vec2* pivot, bool update = true);
#endif