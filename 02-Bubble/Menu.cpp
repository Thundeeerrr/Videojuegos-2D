#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Menu.h"
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include "Game.h"


glm::vec4 calculateMenuUV(float leftX, float rightX, float row, float IMG_W, float IMG_H)
{
	float uvSizeX = (rightX - leftX) / IMG_W;
	float uvSizeY = (IMG_H * 0.5f) / IMG_H;
	float uvOffsetX = leftX / IMG_W;
	float uvOffsetY = (row * (IMG_H * 0.5f)) / IMG_H;

	return glm::vec4(uvSizeX, uvSizeY, uvOffsetX, uvOffsetY);
}

glm::vec4 calculateUV(float leftX, float rightX, float topY, float bottomY, float IMG_W, float IMG_H)
{
	float uvSizeX = (rightX - leftX) / IMG_W;
	float uvSizeY = (bottomY - topY) / IMG_H;
	float uvOffsetX = leftX / IMG_W;
	float uvOffsetY = topY / IMG_H;

	return glm::vec4(uvSizeX, uvSizeY, uvOffsetX, uvOffsetY);
}


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

	cursorSheet.loadFromFile("images/MenuStuff.png", TEXTURE_PIXEL_FORMAT_RGBA);
	cursorSheet.setWrapS(GL_CLAMP_TO_EDGE);
	cursorSheet.setWrapT(GL_CLAMP_TO_EDGE);
	cursorSheet.setMinFilter(GL_NEAREST);
	cursorSheet.setMagFilter(GL_NEAREST);

 glm::vec4 uvData = calculateMenuUV(325.f, 484.f, 0.f, 779.f, 294.f);
	float uvSizeX = uvData.x;
	float uvSizeY = uvData.y;
	float uvOffsetX = uvData.z;
	float uvOffsetY = uvData.w;

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

	uvData = calculateUV(2.f, 8.f, 2.f, 9.f, 66.f, 30.f);
	uvSizeX = uvData.x;
	uvSizeY = uvData.y;
	uvOffsetX = uvData.z;
	uvOffsetY = uvData.w;

	cursorSprite = Sprite::createSprite(
		glm::ivec2(32, 32),
		glm::vec2(uvSizeX, uvSizeY),
		&cursorSheet,
		&texProgram
	);
	cursorSprite->setNumberAnimations(1);
	cursorSprite->setAnimationSpeed(0, 1);
	cursorSprite->addKeyframe(0, glm::vec2(uvOffsetX, uvOffsetY));
	cursorSprite->changeAnimation(0);

	optionPositions[0] = glm::vec2(450.f, 280.f);
	optionPositions[1] = glm::vec2(450.f, 320.f);
	optionPositions[2] = glm::vec2(450.f, 360.f);
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
