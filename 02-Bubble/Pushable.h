#ifndef _PUSHABLE_INCLUDE
#define _PUSHABLE_INCLUDE

#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Texture.h"
#include "ShaderProgram.h"
#include "TileMap.h"

class Pushable
{
public:
	Pushable();
	~Pushable();

	void init(glm::ivec2 tileMapPos, ShaderProgram &shaderProgram, TileMap *tileMap);
	void update(int deltaTime);
	void render();
	void push(int dir);
	glm::vec2 getPosition() const;
	glm::ivec2 getSize() const;
	bool isMoving() const;
	bool isOnGround() const;

private:
 Texture weightTexture;
	ShaderProgram *shaderProgram;
	GLuint vao, vbo;
	GLint posLocation, texCoordLocation;
	TileMap *map;
	glm::vec2 position;
	glm::ivec2 size = glm::ivec2(16, 16);
 float pushStartSpeed = 96.f;
	float pushCurrentSpeed = 0.f;
	int dirX = 0;
	bool moving = false;
  float remainingPushDistance = 0.f;
	float velocityY = 0.f;
	bool onGround = false;
};

#endif // _PUSHABLE_INCLUDE
