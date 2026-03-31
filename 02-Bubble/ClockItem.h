#ifndef _CLOCK_ITEM_INCLUDE
#define _CLOCK_ITEM_INCLUDE

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Texture.h"
#include "ShaderProgram.h"

class ClockItem
{
public:
	ClockItem();
	~ClockItem();

	void init(const glm::ivec2 &tilePos, ShaderProgram &shaderProgram);
	void render() const;
	void collect();
	bool isCollected() const;
	glm::vec2 getPosition() const;
	glm::ivec2 getSize() const;

private:
	glm::vec2 worldPos;
	glm::ivec2 size;
	bool collected;
	GLuint vao, vbo;
	GLint posLocation, texCoordLocation;
	Texture itemTexture;
	ShaderProgram *shaderProgram;
};

#endif // _CLOCK_ITEM_INCLUDE
