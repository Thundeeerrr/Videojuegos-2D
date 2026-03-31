#include <cmath>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Pushable.h"

namespace
{
	const float GRAVITY_PX_PER_SEC2 = 300.f;
	const int TILE_SIZE_PX = 16;
  const int GROUND_PROBE_OFFSET_PX = 1;
  const int PUSH_DISTANCE_TILES = 2;
	const float PUSH_DISTANCE_PX = float(PUSH_DISTANCE_TILES * TILE_SIZE_PX);
   const float PUSH_DECELERATION_PX_PER_SEC2 = 220.f;
	const float PUSH_STOP_SPEED_PX_PER_SEC = 8.f;
	const int WEIGHT_COL = 0;
	const int WEIGHT_ROW = 0;
}

Pushable::Pushable()
{
  map = NULL;
	shaderProgram = NULL;
	vao = 0;
	vbo = 0;
	posLocation = -1;
	texCoordLocation = -1;
}

Pushable::~Pushable()
{
  if(vao != 0)
		glDeleteVertexArrays(1, &vao);
	if(vbo != 0)
		glDeleteBuffers(1, &vbo);
}

void Pushable::init(glm::ivec2 tileMapPos, ShaderProgram &shaderProgram, TileMap *tileMap)
{
	map = tileMap;
    this->shaderProgram = &shaderProgram;
	weightTexture.loadFromFile("images/items.png", TEXTURE_PIXEL_FORMAT_RGBA);
   weightTexture.setWrapS(GL_CLAMP_TO_EDGE);
	weightTexture.setWrapT(GL_CLAMP_TO_EDGE);
	weightTexture.setMinFilter(GL_NEAREST);
	weightTexture.setMagFilter(GL_NEAREST);
	weightTexture.setMaxMipmapLevel(0);

	int sheetCols = weightTexture.width() / TILE_SIZE_PX;
	int sheetRows = weightTexture.height() / TILE_SIZE_PX;
	if(sheetCols <= 0) sheetCols = 1;
	if(sheetRows <= 0) sheetRows = 1;
    const float halfTexelX = 0.5f / float(weightTexture.width());
	const float halfTexelY = 0.5f / float(weightTexture.height());
	float uvX0 = float(WEIGHT_COL) / float(sheetCols);
	float uvY0 = float(WEIGHT_ROW) / float(sheetRows);
	float uvX1 = uvX0 + (1.f / float(sheetCols));
	float uvY1 = uvY0 + (1.f / float(sheetRows));
	uvX0 += halfTexelX;
	uvY0 += halfTexelY;
	uvX1 -= halfTexelX;
	uvY1 -= halfTexelY;
	float vertices[16] = {
		0.f, 0.f, uvX0, uvY0,
		float(TILE_SIZE_PX), 0.f, uvX1, uvY0,
		float(TILE_SIZE_PX), float(TILE_SIZE_PX), uvX1, uvY1,
		0.f, float(TILE_SIZE_PX), uvX0, uvY1
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	posLocation = shaderProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	texCoordLocation = shaderProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));

	position = glm::vec2(tileMapPos);
}

void Pushable::push(int dir)
{
	if(moving || !onGround)
		return;
	if(dir == 0)
		return;

 dirX = (dir > 0) ? 1 : -1;
	remainingPushDistance = PUSH_DISTANCE_PX;
  pushCurrentSpeed = pushStartSpeed;
	moving = true;
}

void Pushable::update(int deltaTime)
{
   if(map == NULL || shaderProgram == NULL)
		return;

	float dt = float(deltaTime) / 1000.f;
  glm::ivec2 posI(int(position.x), int(position.y));
	glm::ivec2 groundProbePos = posI;
	groundProbePos.y += GROUND_PROBE_OFFSET_PX;
	int groundY = posI.y;
   if(!map->collisionMoveDown(groundProbePos, size, &groundY))
	{
		velocityY += GRAVITY_PX_PER_SEC2 * dt;
		position.y += velocityY * dt;
		onGround = false;
	}
	else
	{
		position.y = float(groundY);
		velocityY = 0.f;
		onGround = true;
	}

	if(onGround && moving)
	{
        if(pushCurrentSpeed <= PUSH_STOP_SPEED_PX_PER_SEC)
		{
			moving = false;
			remainingPushDistance = 0.f;
			pushCurrentSpeed = 0.f;
			return;
		}

		float delta = pushCurrentSpeed * dt;
		if(delta > remainingPushDistance)
			delta = remainingPushDistance;

		posI = glm::ivec2(int(position.x), int(position.y));
		if(dirX > 0)
		{
			if(map->collisionMoveRight(posI, size))
         {
				moving = false;
				remainingPushDistance = 0.f;
				pushCurrentSpeed = 0.f;
			}
			else
            {
				position.x += delta;
               remainingPushDistance -= delta;
			}
		}
		else if(dirX < 0)
		{
			if(map->collisionMoveLeft(posI, size))
         {
				moving = false;
				remainingPushDistance = 0.f;
				pushCurrentSpeed = 0.f;
			}
			else
            {
				position.x -= delta;
               remainingPushDistance -= delta;
			}
		}

		pushCurrentSpeed -= PUSH_DECELERATION_PX_PER_SEC2 * dt;
		if(pushCurrentSpeed < 0.f)
			pushCurrentSpeed = 0.f;

		if(remainingPushDistance <= 0.f)
		{
			moving = false;
			remainingPushDistance = 0.f;
           pushCurrentSpeed = 0.f;
		}
	}

}

void Pushable::render()
{
  if(shaderProgram == NULL)
		return;

	glm::mat4 modelview = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.f));
	shaderProgram->setUniformMatrix4f("modelview", modelview);
	shaderProgram->setUniform2f("texCoordDispl", 0.f, 0.f);
	glEnable(GL_TEXTURE_2D);
	weightTexture.use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisable(GL_TEXTURE_2D);
}

glm::vec2 Pushable::getPosition() const
{
	return position;
}

glm::ivec2 Pushable::getSize() const
{
	return size;
}

bool Pushable::isMoving() const
{
	return moving;
}

bool Pushable::isOnGround() const
{
	return onGround;
}
