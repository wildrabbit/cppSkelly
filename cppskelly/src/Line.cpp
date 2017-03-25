#include "Line.h"
#include "Camera.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include "logUtils.h"

const int NUM_LINE_VAO = 1;
const int LINE_VBO_ATTR_POS = 0;
const int LINE_VBO_ATTR_UV = 2;
const int LINE_VBO_ATTR_COLOR = 1;

const int LINE_FLOATS_PER_VERTEX = 2;
const int LINE_FLOATS_PER_UV = 2;
const int LINE_FLOATS_PER_COLOUR = 4;

const int NUM_VERTICES_PER_QUAD = 4;
const int NUM_TRIANGLES_PER_QUAD = 2;
const int NUM_INDEXES_PER_TRIANGLE = 3;
const char* LINE_SHADER_NAME = "lines_default";

void createSegment(const glm::vec2& a, const glm::vec2& b, LineRenderer& renderer)
{
	int numPoints = 2;
	int numQuads = 1;
	if (numQuads <= 0)
	{
		return;
	}
	if (renderer.colours.size() == 0)
	{
		int numColourValues = numPoints * LINE_FLOATS_PER_COLOUR;
		renderer.colours.resize(numColourValues);
		for (int i = 0; i < 2; ++i)
		{
			std::copy(std::begin(renderer.colourRGBA), std::end(renderer.colourRGBA), renderer.colours.begin() + i * LINE_FLOATS_PER_COLOUR);
		}
	}
	if (renderer.linePointWidths.size() == 0 && renderer.lineWidth > 0)
	{
		renderer.linePointWidths.resize(2, renderer.lineWidth);
	}

	glm::vec2 ab = b - a;
	glm::vec2 normal = { -ab.y, ab.x };
	normal = normalize(normal);
	
	renderer.vertices.resize(numQuads * NUM_VERTICES_PER_QUAD * LINE_FLOATS_PER_VERTEX);
	renderer.vertices[0] = a.x + renderer.linePointWidths[0] * normal.x;
	renderer.vertices[1] = a.y + renderer.linePointWidths[0] * normal.y;
	renderer.vertices[2] = a.x - renderer.linePointWidths[0] * normal.x;
	renderer.vertices[3] = a.y - renderer.linePointWidths[0] * normal.y;
	renderer.vertices[4] = b.x + renderer.linePointWidths[1] * normal.x;
	renderer.vertices[5] = b.y + renderer.linePointWidths[1] * normal.y;
	renderer.vertices[6] = b.x - renderer.linePointWidths[1] * normal.x;
	renderer.vertices[7] = b.y - renderer.linePointWidths[1] * normal.y;

	//renderer.uvs.resize(numQuads * NUM_VERTICES_PER_QUAD * LINE_FLOATS_PER_UV);
	//renderer.uvs[0] = 0.f;
	//renderer.uvs[1] = 0.f;
	//renderer.uvs[2] = 0.f;
	//renderer.uvs[3] = 1.f;
	//renderer.uvs[4] = 1.f;
	//renderer.uvs[5] = 0.f;
	//renderer.uvs[6] = 1.f;
	//renderer.uvs[7] = 1.f;

	renderer.colours.resize(numQuads * NUM_VERTICES_PER_QUAD * LINE_FLOATS_PER_COLOUR);
	renderer.colours[0] = renderer.colourRGBA[0];
	renderer.colours[1] = renderer.colourRGBA[1];
	renderer.colours[2] = renderer.colourRGBA[2];
	renderer.colours[3] = renderer.colourRGBA[3];

	renderer.colours[4] = renderer.colourRGBA[0];
	renderer.colours[5] = renderer.colourRGBA[1];
	renderer.colours[6] = renderer.colourRGBA[2];
	renderer.colours[7] = renderer.colourRGBA[3];

	renderer.colours[8] = renderer.colourRGBA[0];
	renderer.colours[9] = renderer.colourRGBA[1];
	renderer.colours[10] = renderer.colourRGBA[2];
	renderer.colours[11] = renderer.colourRGBA[3];

	renderer.colours[12] = renderer.colourRGBA[0];
	renderer.colours[13] = renderer.colourRGBA[1];
	renderer.colours[14] = renderer.colourRGBA[2];
	renderer.colours[15] = renderer.colourRGBA[3];

	renderer.indexes.resize(numQuads * NUM_TRIANGLES_PER_QUAD * NUM_INDEXES_PER_TRIANGLE);
	renderer.indexes[0] = 1;
	renderer.indexes[1] = 2;
	renderer.indexes[2] = 0;
	renderer.indexes[3] = 1;
	renderer.indexes[4] = 3;
	renderer.indexes[5] = 2;
}

void initGeometry(LineRenderer& renderer)
{
		glCreateVertexArrays(NUM_LINE_VAO, &(renderer.vaoID));
		glBindVertexArray(renderer.vaoID); // current vtex array

		glCreateBuffers(NUM_LINE_VBO, renderer.vboIDs);

		// pos
		glBindBuffer(GL_ARRAY_BUFFER, renderer.vboIDs[LINE_VBO_ATTR_POS]);
		glBufferData(GL_ARRAY_BUFFER, renderer.vertices.size() * sizeof(GLfloat), renderer.vertices.data(), GL_DYNAMIC_DRAW);

		glVertexAttribPointer(LINE_VBO_ATTR_POS, LINE_FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0)); // Coord. info => Atr. index #0, three floats/vtx
		glEnableVertexAttribArray(LINE_VBO_ATTR_POS);


		//// UVS
		//glBindBuffer(GL_ARRAY_BUFFER, renderer.vboIDs[LINE_VBO_ATTR_UV]);
		//glBufferData(GL_ARRAY_BUFFER, (NUM_VERTICES_PER_QUAD * LINE_FLOATS_PER_UV) * sizeof(GLfloat), (void*)&(renderer.uvs), GL_DYNAMIC_DRAW);

		//glVertexAttribPointer(LINE_VBO_ATTR_UV, LINE_FLOATS_PER_UV, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		//glEnableVertexAttribArray(LINE_VBO_ATTR_UV);


		// Colours
		glBindBuffer(GL_ARRAY_BUFFER, renderer.vboIDs[LINE_VBO_ATTR_COLOR]);
		glBufferData(GL_ARRAY_BUFFER, renderer.colours.size() * sizeof(GLfloat), renderer.colours.data(), GL_DYNAMIC_DRAW);

		glVertexAttribPointer(LINE_VBO_ATTR_COLOR, LINE_FLOATS_PER_COLOUR, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glEnableVertexAttribArray(LINE_VBO_ATTR_COLOR);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Indexes
		glCreateBuffers(1, &renderer.eboID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.eboID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer.indexes.size() * sizeof(GLuint), renderer.indexes.data(), GL_STATIC_DRAW);
	
}
void updateLineSprite (LineRenderer& line)
{
	line.modelMatrix = glm::translate(glm::vec3(line.pos.x, line.pos.y, 0.0f))
		* glm::rotate(line.angle, glm::vec3(0.0f, 0.0f, 1.0f))
		* glm::scale(glm::vec3(line.scale.x, line.scale.y, 1.0f));
}

void cleanupLine(LineRenderer& line)
{
	glDisableVertexAttribArray(0);
	glDeleteBuffers(NUM_LINE_VBO, line.vboIDs);
	glDeleteBuffers(1, &line.eboID);
	glDeleteVertexArrays(NUM_LINE_VAO, &line.vaoID);
	//glDeleteSamplers(1, &line.samplerID);
}

void LineRenderer::draw(SDL_Window* w, Camera* c)
{
	glm::mat4 mvp = c->projMatrix * c->viewMatrix * modelMatrix;
	// Pass matrices, setup shader params, etc
	TShaderTableIter shaderIt = gShaders.find(shaderName);
	if (shaderIt == gShaders.end())
	{
		logError("Shader not found!!");
		return;
	}

	Shader shader = shaderIt->second;
	shader.bindAttributeLocation(LINE_VBO_ATTR_POS, "inPos");
	//shader.bindAttributeLocation(LINE_VBO_ATTR_UV, "inTexCoord");
	shader.bindAttributeLocation(LINE_VBO_ATTR_COLOR, "inColour");

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
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[LINE_VBO_ATTR_POS]);
	glVertexAttribPointer(LINE_VBO_ATTR_POS, LINE_FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, 0); // Coord. info => Atr. index #0, three floats/vtx
	glEnableVertexAttribArray(LINE_VBO_ATTR_POS);
	//glBindBuffer(GL_ARRAY_BUFFER, vboIDs[LINE_VBO_ATTR_UV]);
	//glVertexAttribPointer(LINE_VBO_ATTR_UV, LINE_FLOATS_PER_UV, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(LINE_VBO_ATTR_UV);
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[LINE_VBO_ATTR_COLOR]);
	glVertexAttribPointer(LINE_VBO_ATTR_COLOR, LINE_FLOATS_PER_COLOUR, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(LINE_VBO_ATTR_COLOR);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);

	//glDrawArrays(GL_TRIANGLES, 0, NUM_SPRITE_TRIANGLES_VERT_COUNT);
	glDrawElements(GL_TRIANGLES, (GLsizei)indexes.size(), GL_UNSIGNED_INT, nullptr);
	//glDrawArrays(GL_TRIANGLES, 0, NUM_SPRITE_TRIANGLES_VERT_COUNT);
}

void updateGeometry(LineRenderer& renderer)
{
	glBindVertexArray(renderer.vaoID); // current vtex array
									  // pos
	glBindBuffer(GL_ARRAY_BUFFER, renderer.vboIDs[LINE_VBO_ATTR_POS]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, renderer.vertices.size(), renderer.vertices.data());
	glVertexAttribPointer(LINE_VBO_ATTR_POS, LINE_FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, 0); // Coord. info => Atr. index #0, three floats/vtx

	//glBindBuffer(GL_ARRAY_BUFFER, renderer.vboIDs[LINE_VBO_ATTR_UV]);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(renderer.uvs), &renderer.uvs);
	//glVertexAttribPointer(LINE_VBO_ATTR_UV, LINE_FLOATS_PER_UV, GL_FLOAT, GL_FALSE, 0, 0); // Coord. info => Atr. index #0, 2 floats/UV

	glBindBuffer(GL_ARRAY_BUFFER, renderer.vboIDs[LINE_VBO_ATTR_COLOR]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, renderer.colours.size(), renderer.colours.data());
	glVertexAttribPointer(LINE_VBO_ATTR_COLOR, LINE_FLOATS_PER_COLOUR, GL_FLOAT, GL_FALSE, 0, 0); // Coord. info => Atr. index #0, 4 floats/Col
}

void LineRenderer::cleanup()
{
	glDisableVertexAttribArray(0);
	glDeleteBuffers(NUM_LINE_VBO, vboIDs);
	glDeleteBuffers(1, &eboID);
	glDeleteVertexArrays(NUM_LINE_VAO, &vaoID);
	//glDeleteSamplers(1, &samplerID);

}
