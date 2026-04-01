#ifndef _DOOR_INCLUDE
#define _DOOR_INCLUDE

#include <glm/glm.hpp>
#include "Texture.h"
#include "ShaderProgram.h"

class Door
{
public:
 enum class Type { Normal, LockedExit };

	Door();
	~Door();

	void init(const glm::ivec2 &tilePos, Type type, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render() const;
	void toggleOpen();
	void open();
	bool getIsOpen() const { return isOpen; }
	bool isLockedExit() const { return type == Type::LockedExit; }
	glm::ivec2 getTilePos() const { return tilePos; }

private:
	void updateUvsForRow(int rowIndex);

private:
	glm::vec2 worldPos;
	glm::ivec2 tilePos;
	Type type;
	bool isOpen;
	GLuint vao, vbo;
	GLint posLocation, texCoordLocation;
	Texture doorTexture;
	ShaderProgram *shaderProgram;
};

#endif // _DOOR_INCLUDE
