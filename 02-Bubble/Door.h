#ifndef _DOOR_INCLUDE
#define _DOOR_INCLUDE

#include <glm/glm.hpp>
#include "Texture.h"
#include "ShaderProgram.h"

class Door
{
public:
	Door();
	~Door();

	void init(const glm::ivec2 &tilePos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render() const;
	void toggleOpen();

private:
	void updateUvs(float uvY0, float uvY1);

private:
	glm::vec2 worldPos;
	glm::ivec2 tilePos;
	bool isOpen;
	GLuint vao, vbo;
	GLint posLocation, texCoordLocation;
	Texture doorTexture;
	ShaderProgram *shaderProgram;
};

#endif // _DOOR_INCLUDE
