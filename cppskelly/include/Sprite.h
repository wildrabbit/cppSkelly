#include "glad\glad.h"
#include <glm\glm.hpp>

class Sprite
{
private:
	glm::mat4 mMatrix;
public:
	glm::vec2 mPos;

	GLuint mTextureID;
};