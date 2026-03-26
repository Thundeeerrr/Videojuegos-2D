#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Credits.h"
#include "Game.h"

using namespace std;

static glm::vec4 calculateUV(float leftX, float rightX, float topY, float bottomY,
	                  float IMG_W, float IMG_H)
{
	return glm::vec4(
		(rightX - leftX) / IMG_W,
		(bottomY - topY) / IMG_H,
		leftX / IMG_W,
		topY / IMG_H
	);
}

Credits::Credits() : slide(nullptr), alpha(0.f), timer(0.f), finished(false), phase(FADE_IN)
{
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
	alpha = 0.f;
	timer = 0.f;
	finished = false;
	phase = FADE_IN;

	sheet.loadFromFile("images/MainMenu.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sheet.setWrapS(GL_CLAMP_TO_EDGE);
	sheet.setWrapT(GL_CLAMP_TO_EDGE);
	sheet.setMinFilter(GL_NEAREST);
	sheet.setMagFilter(GL_NEAREST);

	const float IMG_W = 779.f, IMG_H = 294.f;
	const float LEFT_X = 163.f;
	const float RIGHT_X = 322.f;
	const float TOP_Y = 2.f;
	const float BOT_Y = 145.f;
	glm::vec4 uv = calculateUV(LEFT_X, RIGHT_X, TOP_Y, BOT_Y, IMG_W, IMG_H);

	if(slide != nullptr)
		delete slide;

	slide = Sprite::createSprite(
		glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT),
		glm::vec2(uv.x, uv.y),
		&sheet, &texProgram
	);
	slide->setNumberAnimations(1);
	slide->setAnimationSpeed(0, 1);
	slide->addKeyframe(0, glm::vec2(uv.z, uv.w));
	slide->changeAnimation(0);
	slide->setPosition(glm::vec2(0.f, 0.f));
}

void Credits::update(int deltaTime)
{
	if(finished) return;
	float dt = deltaTime / 1000.f;
	timer += dt;

	switch(phase)
	{
	case FADE_IN:
		alpha = glm::clamp(timer / FADE_IN_TIME, 0.f, 1.f);
		if(timer >= FADE_IN_TIME)
		{
			alpha = 1.f;
			phase = HOLD;
			timer = 0.f;
		}
		break;
	case HOLD:
		if(timer >= HOLD_TIME)
		{
			phase = FADE_OUT;
			timer = 0.f;
		}
		break;
	case FADE_OUT:
		alpha = glm::clamp(1.f - timer / FADE_OUT_TIME, 0.f, 1.f);
		if(timer >= FADE_OUT_TIME)
		{
			alpha = 0.f;
			finished = true;
		}
		break;
	}
}

void Credits::render()
{
	if(finished) return;

	glm::mat4 modelview(1.f);
	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.f, 1.f, 1.f, alpha);
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
