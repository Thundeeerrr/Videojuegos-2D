#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Bomb.h"

namespace
{
	const int BOMB_TILE_SIZE_PX = 16;
	const int ITEMS_SHEET_COLS = 5;
	const int ITEMS_SHEET_ROWS = 1;
	const int BOMB_INACTIVE_COL = 3;
	const int BOMB_ACTIVE_COL = 4;
	const int BOMB_ROW = 0;
	const int BOMB_PICKUP_COOLDOWN_MS = 250;
}

Bomb::Bomb()
{
	worldPos = glm::vec2(0.f);
	size = glm::ivec2(BOMB_TILE_SIZE_PX, BOMB_TILE_SIZE_PX);
	collected = false;
	armed = false;
	activated = false;
	pickupCooldownMs = 0;
	vao = 0;
	vbo = 0;
	posLocation = -1;
	texCoordLocation = -1;
	shaderProgram = NULL;
}

Bomb::~Bomb()
{
	if(vao != 0)
		glDeleteVertexArrays(1, &vao);
	if(vbo != 0)
		glDeleteBuffers(1, &vbo);
}

void Bomb::init(const glm::ivec2 &tilePos, ShaderProgram &shaderProgram)
{
	this->shaderProgram = &shaderProgram;
	worldPos = glm::vec2(float(tilePos.x * BOMB_TILE_SIZE_PX), float(tilePos.y * BOMB_TILE_SIZE_PX));
	collected = false;
	armed = false;
	activated = false;
	pickupCooldownMs = 0;

	bombTexture.loadFromFile("images/items.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bombTexture.setWrapS(GL_CLAMP_TO_EDGE);
	bombTexture.setWrapT(GL_CLAMP_TO_EDGE);
	bombTexture.setMinFilter(GL_NEAREST);
	bombTexture.setMagFilter(GL_NEAREST);
	bombTexture.setMaxMipmapLevel(0);

	const float tileW = 1.f / float(ITEMS_SHEET_COLS);
	const float tileH = 1.f / float(ITEMS_SHEET_ROWS);
	const float uvX0 = 0.f;
	const float uvY0 = float(BOMB_ROW) * tileH;
	const float uvX1 = uvX0 + tileW;
	const float uvY1 = uvY0 + tileH;

	float vertices[16] = {
		0.f, 0.f, uvX0, uvY0,
		float(BOMB_TILE_SIZE_PX), 0.f, uvX1, uvY0,
		float(BOMB_TILE_SIZE_PX), float(BOMB_TILE_SIZE_PX), uvX1, uvY1,
		0.f, float(BOMB_TILE_SIZE_PX), uvX0, uvY1
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	posLocation = shaderProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	texCoordLocation = shaderProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));
}

void Bomb::update(int deltaTime)
{
	if(pickupCooldownMs > 0)
	{
		pickupCooldownMs -= deltaTime;
		if(pickupCooldownMs < 0)
			pickupCooldownMs = 0;
	}
}

void Bomb::render() const
{
	if(shaderProgram == NULL || collected)
		return;

	const float tileW = 1.f / float(ITEMS_SHEET_COLS);
	const int col = activated ? BOMB_ACTIVE_COL : BOMB_INACTIVE_COL;
	const float texDisplX = float(col) * tileW;

	glm::mat4 modelview = glm::translate(glm::mat4(1.0f), glm::vec3(worldPos.x, worldPos.y, 0.f));
	shaderProgram->setUniformMatrix4f("modelview", modelview);
	shaderProgram->setUniform2f("texCoordDispl", texDisplX, 0.f);
	glEnable(GL_TEXTURE_2D);
	bombTexture.use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisable(GL_TEXTURE_2D);
}

void Bomb::collect()
{
	if(collected)
		return;

	collected = true;
	armed = false;
	activated = true;
}

void Bomb::dropAt(const glm::ivec2 &worldPos)
{
	this->worldPos = glm::vec2(float(worldPos.x), float(worldPos.y));
	collected = false;
	armed = true;
	activated = true;
	pickupCooldownMs = BOMB_PICKUP_COOLDOWN_MS;
}

void Bomb::onEnemyTouch()
{
	armed = false;
	pickupCooldownMs = BOMB_PICKUP_COOLDOWN_MS;
}

bool Bomb::isCollected() const
{
	return collected;
}

bool Bomb::isArmed() const
{
	return armed;
}

bool Bomb::canBeCollected() const
{
	return !collected && pickupCooldownMs == 0;
}

bool Bomb::hasBeenActivated() const
{
	return activated;
}

glm::vec2 Bomb::getPosition() const
{
	return worldPos;
}

glm::ivec2 Bomb::getSize() const
{
	return size;
}
