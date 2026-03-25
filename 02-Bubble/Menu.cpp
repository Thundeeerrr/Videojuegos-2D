#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Menu.h"
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include "Game.h"


Menu::Menu()
{
	backgroundSprite = NULL;
	cursorSprite = NULL;
	selectedOption = 0;
}

Menu::~Menu()
{
	if(backgroundSprite != NULL)
		delete backgroundSprite;
	if(cursorSprite != NULL)
		delete cursorSprite;
	texProgram.free();
}

void Menu::init()
{
	initShaders();
	projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
	selectedOption = 0;

	menuSheet.loadFromFile("images/MainMenu.png", TEXTURE_PIXEL_FORMAT_RGBA);
	menuSheet.setWrapS(GL_CLAMP_TO_EDGE);
	menuSheet.setWrapT(GL_CLAMP_TO_EDGE);
	menuSheet.setMinFilter(GL_NEAREST);
	menuSheet.setMagFilter(GL_NEAREST);

	// Dimensiones de la imagen
	const float IMG_W = 779.f;
	const float IMG_H = 294.f;

	// ← Solo cambia estos dos valores (píxel X izquierdo y derecho del trozo)
	const float BLUE_X_LEFT = 325.f;
	const float BLUE_X_RIGHT = 484.f;
	// Si quieres la fila de abajo, cambia ROW a 1
	const float ROW = 0.f;

	// Cálculos automáticos
	float uvSizeX = (BLUE_X_RIGHT - BLUE_X_LEFT) / IMG_W;
	float uvSizeY = 0.5f;  // siempre 0.5 porque hay 2 filas
	float uvOffsetX = BLUE_X_LEFT / IMG_W;
	float uvOffsetY = ROW * 0.5f;

	backgroundSprite = Sprite::createSprite(
		glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT),
		glm::vec2(uvSizeX, uvSizeY),
		&menuSheet,
		&texProgram
	);
	backgroundSprite->setNumberAnimations(1);
	backgroundSprite->setAnimationSpeed(0, 1);
	backgroundSprite->addKeyframe(0, glm::vec2(uvOffsetX, uvOffsetY));
	backgroundSprite->changeAnimation(0);
	backgroundSprite->setPosition(glm::vec2(0.f, 0.f));

	cursorSprite = Sprite::createSprite(
		glm::ivec2(16, 16),
		glm::vec2(0.02f, 0.034f),
		&menuSheet,
		&texProgram
	);
	cursorSprite->setNumberAnimations(1);
	cursorSprite->setAnimationSpeed(0, 1);
	cursorSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	cursorSprite->changeAnimation(0);

	optionPositions[0] = glm::vec2(260.f, 280.f);
	optionPositions[1] = glm::vec2(260.f, 320.f);
	optionPositions[2] = glm::vec2(260.f, 360.f);
}

void Menu::update(int deltaTime)
{
}

void Menu::render()
{
	glm::mat4 modelview(1.f);

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	texProgram.setUniformMatrix4f("modelview", modelview);

	backgroundSprite->render();

	cursorSprite->setPosition(optionPositions[selectedOption] - glm::vec2(24.f, 0.f));
	cursorSprite->render();
}

void Menu::keyPressed(int key)
{
	if(key == GLFW_KEY_DOWN)
		selectedOption = (selectedOption + 1) % NUM_OPTIONS;
	else if(key == GLFW_KEY_UP)
		selectedOption = (selectedOption + NUM_OPTIONS - 1) % NUM_OPTIONS;
	else if(key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER)
	{
		if(selectedOption == 0)
			Game::instance().changeState(STATE_PLAYING);
		else if(selectedOption == 1)
			Game::instance().changeState(STATE_INSTRUCTIONS);
		else if(selectedOption == 2)
			Game::instance().changeState(STATE_CREDITS);
	}
}

void Menu::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if(!texProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << texProgram.log() << endl << endl;
	}
	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}
