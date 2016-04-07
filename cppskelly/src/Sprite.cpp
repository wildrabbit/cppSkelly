#include "Sprite.h"

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