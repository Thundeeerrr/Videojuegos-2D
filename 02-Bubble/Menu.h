#ifndef _MENU_INCLUDE
#define _MENU_INCLUDE


#include <GL/glew.h>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "Texture.h"
#include "Sprite.h"


class Menu
{

public:
	Menu();
	~Menu();

	void init();
	void update(int deltaTime);
	void render();
	void keyPressed(int key);

private:
	void initShaders();

private:
	ShaderProgram texProgram;
	glm::mat4 projection;

	Texture menuSheet;
	Sprite *backgroundSprite;
	Sprite *cursorSprite;

	int selectedOption;
	static const int NUM_OPTIONS = 3;
	glm::vec2 optionPositions[NUM_OPTIONS];

};


#endif // _MENU_INCLUDE
