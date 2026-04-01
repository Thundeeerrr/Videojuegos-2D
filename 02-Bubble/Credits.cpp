#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Credits.h"
#include "Game.h"

using namespace std;

Credits::Credits()
{
	slide = NULL;
	finished = false;
   elapsedTimeMs = 0;
}

Credits::~Credits()
{
	if(slide != nullptr)
		delete slide;
	texProgram.free();
}

void Credits::init()
{
	initShaders();
	projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
	finished = false;
	elapsedTimeMs = 0;

   sheet.loadFromFile("images/Credits.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sheet.setWrapS(GL_CLAMP_TO_EDGE);
	sheet.setWrapT(GL_CLAMP_TO_EDGE);
	sheet.setMinFilter(GL_NEAREST);
	sheet.setMagFilter(GL_NEAREST);

	if(slide != nullptr)
		delete slide;

	slide = Sprite::createSprite(
		glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT),
      glm::vec2(1.f, 1.f),
		&sheet, &texProgram
	);
	slide->setNumberAnimations(1);
	slide->setAnimationSpeed(0, 1);
   slide->addKeyframe(0, glm::vec2(0.f, 0.f));
	slide->changeAnimation(0);
	slide->setPosition(glm::vec2(0.f, 0.f));
}

void Credits::update(int deltaTime)
{
   if(finished)
		return;

	elapsedTimeMs += deltaTime;
	if(elapsedTimeMs >= DURATION_MS)
		finished = true;
}

void Credits::render()
{
	glm::mat4 modelview(1.f);
	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
 texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	slide->render();
}

void Credits::initShaders()
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
