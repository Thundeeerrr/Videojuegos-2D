#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "ShieldItem.h"

namespace
{
	const int ITEM_TILE_SIZE_PX = 16;
	const int ITEMS_SHEET_COLS = 5;
	const int ITEMS_SHEET_ROWS = 1;
	const int SHIELD_ITEM_COL = 1;
	const int SHIELD_ITEM_ROW = 0;
}

ShieldItem::ShieldItem()
{
	worldPos = glm::vec2(0.f);
	size = glm::ivec2(ITEM_TILE_SIZE_PX, ITEM_TILE_SIZE_PX);
	collected = false;
	vao = 0;
	vbo = 0;
	posLocation = -1;
	texCoordLocation = -1;
	shaderProgram = NULL;
}

ShieldItem::~ShieldItem()
{
	if(vao != 0)
		glDeleteVertexArrays(1, &vao);
	if(vbo != 0)
		glDeleteBuffers(1, &vbo);
}

void ShieldItem::init(const glm::ivec2 &tilePos, ShaderProgram &shaderProgram)
{
	this->shaderProgram = &shaderProgram;
	worldPos = glm::vec2(float(tilePos.x * ITEM_TILE_SIZE_PX), float(tilePos.y * ITEM_TILE_SIZE_PX));
	collected = false;

	itemTexture.loadFromFile("images/items.png", TEXTURE_PIXEL_FORMAT_RGBA);
	itemTexture.setWrapS(GL_CLAMP_TO_EDGE);
	itemTexture.setWrapT(GL_CLAMP_TO_EDGE);
	itemTexture.setMinFilter(GL_NEAREST);
	itemTexture.setMagFilter(GL_NEAREST);
	itemTexture.setMaxMipmapLevel(0);

	const float tileW = 1.f / float(ITEMS_SHEET_COLS);
	const float tileH = 1.f / float(ITEMS_SHEET_ROWS);
	const float uvX0 = 0.f;
	const float uvY0 = float(SHIELD_ITEM_ROW) * tileH;
	const float uvX1 = uvX0 + tileW;
	const float uvY1 = uvY0 + tileH;

	float vertices[16] = {
		0.f, 0.f, uvX0, uvY0,
		float(ITEM_TILE_SIZE_PX), 0.f, uvX1, uvY0,
		float(ITEM_TILE_SIZE_PX), float(ITEM_TILE_SIZE_PX), uvX1, uvY1,
		0.f, float(ITEM_TILE_SIZE_PX), uvX0, uvY1
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	posLocation = shaderProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	texCoordLocation = shaderProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));
}

void ShieldItem::render() const
{
	if(shaderProgram == NULL || collected)
		return;

	const float tileW = 1.f / float(ITEMS_SHEET_COLS);
	const float texDisplX = float(SHIELD_ITEM_COL) * tileW;

	glm::mat4 modelview = glm::translate(glm::mat4(1.0f), glm::vec3(worldPos.x, worldPos.y, 0.f));
	shaderProgram->setUniformMatrix4f("modelview", modelview);
	shaderProgram->setUniform2f("texCoordDispl", texDisplX, 0.f);
	glEnable(GL_TEXTURE_2D);
	itemTexture.use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisable(GL_TEXTURE_2D);
}

void ShieldItem::collect()
{
	collected = true;
}

bool ShieldItem::isCollected() const
{
	return collected;
}

glm::vec2 ShieldItem::getPosition() const
{
	return worldPos;
}

glm::ivec2 ShieldItem::getSize() const
{
	return size;
}
