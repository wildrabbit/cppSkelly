#pragma once
#ifndef LINEH_H
#define LINEH_H

#include <string>
#include <glad\glad.h>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include "Drawable.h"

static const int NUM_LINE_VBO = 2;
extern const int NUM_LINE_VAO;
extern const int LINE_VBO_ATTR_POS;
extern const int LINE_VBO_ATTR_UV;
extern const int LINE_VBO_ATTR_COLOR;

extern const int LINE_FLOATS_PER_VERTEX;
extern const int LINE_FLOATS_PER_UV;
extern const int LINE_FLOATS_PER_COLOUR;

extern const int NUM_VERTICES_PER_QUAD;
extern const int NUM_TRIANGLES_PER_QUAD;
extern const int NUM_INDEXES_PER_TRIANG;


extern const char* LINE_SHADER_NAME;

struct SDL_Window;
struct Camera;

struct LineRenderer: public Drawable
{
	std::string name;
	std::string texPath;
	std::string shaderName;
	unsigned int texID;
	unsigned int shaderID;
	unsigned int samplerID;
	
	glm::vec2 pos = { 0.f,0.f };
	glm::vec2 scale = { 1.f,1.f }; // Will this make sense?
	float angle = 0.f;

	float speed;

	glm::vec2 velocity;
	glm::vec2 acceleration;

	glm::vec2 pivot;
	glm::mat4 modelMatrix;

	bool alphaBlend;

	std::vector<GLfloat> vertices;
	std::vector<GLfloat> colours;
	std::vector<GLfloat> uvs;
	std::vector<GLuint> indexes;

	float lineWidth;
	GLfloat colourRGBA[4];
	std::vector<float> linePointWidths;

	unsigned int vaoID;
	unsigned int vboIDs[NUM_LINE_VBO];
	unsigned int eboID;

	void draw(SDL_Window* w, Camera* c) override;
	void cleanup() override;

	LineRenderer(const std::string& name)
		:name(name)
		, texPath(), shaderName()
		, texID(0), shaderID(0), samplerID(0)
		, angle(0.0f)
		, pos(), scale(1.0f, 1.0f)
		, pivot(), modelMatrix(), alphaBlend(false)
	{}
};

void initGeometry(LineRenderer& renderer);

void createSegment(const glm::vec2& a, const glm::vec2& b, LineRenderer& renderer);
void createPolyline(const std::vector<glm::vec2>& points, LineRenderer& renderer);

void addPoints(const std::vector<glm::vec2&>& points, LineRenderer& renderer);
void createQuadraticBezier(const glm::vec2& a, const glm::vec2& b, const glm::vec2& control, LineRenderer& renderer, int numSteps);
void createCubicBezier(const glm::vec2& a, const glm::vec2& b, const glm::vec2& control1, const glm::vec2& control2, LineRenderer& renderer, int numSteps);
void updateGeometry(LineRenderer& renderer);

#endif