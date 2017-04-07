#include "Sprite.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "logUtils.h"

const int NUM_SPRITE_VAO = 1;
const int SPRITE_VBO_ATTR_POS = 0;
const int SPRITE_VBO_ATTR_UV = 1;


const char* DEFAULT_SHADER_NAME = "sprites_default";

//Define this somewhere in your header file
#define BUFFER_OFFSET(i) ((void*)(i))


Sprite::Sprite(const std::string& name)
	:name(name)
	, texPath(), shaderName()
	, texID(0), shaderID(0), samplerID(0)
	, clipRect()
	, width(0.0f), height(0.0f), angle(0.0f)
	, pos(), scale(1.0f, 1.0f)
	, pivot(), modelMatrix(), alphaBlend(false)
{}

Sprite::~Sprite()
{

}

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

void Sprite::cleanup()
{
	glDisableVertexAttribArray(SPRITE_VBO_ATTR_POS);
	glDisableVertexAttribArray(SPRITE_VBO_ATTR_UV);
	glDeleteBuffers(NUM_SPRITE_VBO, vboIDs);
	glDeleteBuffers(1, &eboID);
	glDeleteVertexArrays(NUM_SPRITE_VAO, &vaoID);
	glDeleteSamplers(1, &samplerID);
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (NUM_SPRITE_TRIANGLES_IDX_COUNT) * sizeof(GLuint), sprite->indexes, GL_STATIC_DRAW);


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

void setCustomPivot(Sprite* sprite, glm::vec2* pivot, bool update)
{
	sprite->pivot.x = pivot->x;
	sprite->pivot.y = pivot->y;
	if (update)
		updateGeometry(sprite);
}

void setPivotType(Sprite* sprite, PivotType pivotType, bool update)
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
