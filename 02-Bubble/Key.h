#ifndef _KEY_INCLUDE
#define _KEY_INCLUDE

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Texture.h"
#include "ShaderProgram.h"

class Key
{
public:
	Key();
	~Key();

	void init(const glm::ivec2 &tilePos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render() const;
	void collect();
	bool isCollected() const { return collected; }
	glm::ivec2 getTilePos() const { return tilePos; }

private:
	glm::vec2 worldPos;
	glm::ivec2 tilePos;
	bool collected;
	GLuint vao, vbo;
	GLint posLocation, texCoordLocation;
	Texture keyTexture;
	ShaderProgram *shaderProgram;
};

#endif // _KEY_INCLUDE
