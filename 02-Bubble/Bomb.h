#ifndef _BOMB_INCLUDE
#define _BOMB_INCLUDE

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Texture.h"
#include "ShaderProgram.h"

class Bomb
{
public:
	Bomb();
	~Bomb();

	void init(const glm::ivec2 &tilePos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render() const;

	void collect();
	void dropAt(const glm::ivec2 &worldPos);
	void onEnemyTouch();

	bool isCollected() const;
	bool isArmed() const;
	bool canBeCollected() const;
	bool hasBeenActivated() const;
	glm::vec2 getPosition() const;
	glm::ivec2 getSize() const;

private:
	glm::vec2 worldPos;
	glm::ivec2 size;
	bool collected;
	bool armed;
	bool activated;
	int pickupCooldownMs;

	GLuint vao, vbo;
	GLint posLocation, texCoordLocation;
	Texture bombTexture;
	ShaderProgram *shaderProgram;
};

#endif // _BOMB_INCLUDE
