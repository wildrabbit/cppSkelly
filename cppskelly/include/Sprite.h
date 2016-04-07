#ifndef SPRITEH_H
#define SPRITEH_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "GeomUtils.h"
#include "glad/glad.h"

// Forget about batching for now
const int NUM_SPRITE_VBO = 2;
const int NUM_SPRITE_VAO = 1;
const int NUM_SPRITE_TRIANGLES_VERT_COUNT = 4;
const int NUM_SPRITE_TRIANGLES_IDX_COUNT = 6;
const int SPRITE_VBO_ATTR_POS = 0;
const int SPRITE_VBO_ATTR_UV = 1;

const int SPRITE_FLOATS_PER_VERTEX = 2;
const int SPRITE_FLOATS_PER_UV = 2;

class Camera;
class OrthoCamera;

class Sprite
{
private:
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

		void initGeometry();
		void updateMatrix();
		void updateGeometry();
public:
		Rect clipRect;

		glm::vec2 pos;
		glm::vec2 scale;
		float width;
		float height;
		float angle;

		bool alphaBlend;

		Sprite(const std::string& name);
		virtual ~Sprite();

		void init(const std::string& texPath, const std::string& shaderName, const glm::vec2& position, const glm::vec2& dimensions, const glm::vec2& scale, float angle, bool alphaBlend);
		void cleanup();

		void render(const Camera* cam);
		void render(const OrthoCamera* cam);
		void update(float dt);

		void setCustomPivot(const glm::vec2& pivot, bool update = true);
		void setPivotType(PivotType pt, bool update = true);
};

#endif