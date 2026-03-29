#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Instructions.h"
#include "Game.h"

using namespace std;

Instructions::Instructions()
{
	instructionsSprite = NULL;
}

Instructions::~Instructions()
{
	if(instructionsSprite != NULL)
		delete instructionsSprite;
	texProgram.free();
}

void Instructions::init()
{
	initShaders();
	projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);

	instructionsSheet.loadFromFile("images/Texto.png", TEXTURE_PIXEL_FORMAT_RGBA);
	instructionsSheet.setWrapS(GL_CLAMP_TO_EDGE);
	instructionsSheet.setWrapT(GL_CLAMP_TO_EDGE);
	instructionsSheet.setMinFilter(GL_NEAREST);
	instructionsSheet.setMagFilter(GL_NEAREST);

	if(instructionsSprite != NULL)
		delete instructionsSprite;

	instructionsSprite = Sprite::createSprite(
		glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT),
		glm::vec2(1.f, 1.f),
		&instructionsSheet,
		&texProgram
	);
	instructionsSprite->setNumberAnimations(1);
	instructionsSprite->setAnimationSpeed(0, 1);
	instructionsSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	instructionsSprite->changeAnimation(0);
	instructionsSprite->setPosition(glm::vec2(0.f, 0.f));
}

void Instructions::update(int deltaTime)
{
}

void Instructions::render()
{
	glm::mat4 modelview(1.f);

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	texProgram.setUniformMatrix4f("modelview", modelview);

	instructionsSprite->render();
}

void Instructions::initShaders()
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
