#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Door.h"

namespace
{
	const int TILE_SIZE = 16;
	const int DOOR_WIDTH = 16;
	const int DOOR_HEIGHT = 24;
	const int DOOR_OFFSET_Y = 8;
}

Door::Door()
{
	worldPos = glm::vec2(0.f);
	tilePos = glm::ivec2(0);
	isOpen = false;
	vao = 0;
	vbo = 0;
	posLocation = -1;
	texCoordLocation = -1;
	shaderProgram = NULL;
}

Door::~Door()
{
	if(vao != 0)
		glDeleteVertexArrays(1, &vao);
	if(vbo != 0)
		glDeleteBuffers(1, &vbo);
}

void Door::init(const glm::ivec2 &tilePos, ShaderProgram &shaderProgram)
{
	this->tilePos = tilePos;
	this->shaderProgram = &shaderProgram;
	isOpen = false;

	worldPos.x = float(tilePos.x * TILE_SIZE);
	worldPos.y = float(tilePos.y * TILE_SIZE - DOOR_OFFSET_Y);

	doorTexture.loadFromFile("images/doors.png", TEXTURE_PIXEL_FORMAT_RGBA);
	doorTexture.setWrapS(GL_CLAMP_TO_EDGE);
	doorTexture.setWrapT(GL_CLAMP_TO_EDGE);
	doorTexture.setMinFilter(GL_NEAREST);
	doorTexture.setMagFilter(GL_NEAREST);

	const float uvY0 = 0.5f;
	const float uvY1 = 1.f;
	float vertices[16] = {
		0.f, 0.f, 0.f, uvY0,
		float(DOOR_WIDTH), 0.f, 1.f, uvY0,
		float(DOOR_WIDTH), float(DOOR_HEIGHT), 1.f, uvY1,
		0.f, float(DOOR_HEIGHT), 0.f, uvY1
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	posLocation = shaderProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	texCoordLocation = shaderProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));
}

void Door::update(int deltaTime)
{
}

void Door::render() const
{
	glm::mat4 modelview = glm::translate(glm::mat4(1.0f), glm::vec3(worldPos.x, worldPos.y, 0.f));
	shaderProgram->setUniformMatrix4f("modelview", modelview);
	shaderProgram->setUniform2f("texCoordDispl", 0.f, 0.f);
	glEnable(GL_TEXTURE_2D);
	doorTexture.use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisable(GL_TEXTURE_2D);
}

void Door::toggleOpen()
{
	isOpen = !isOpen;
	if(isOpen)
		updateUvs(0.f, 0.5f);
	else
		updateUvs(0.5f, 1.f);
}

void Door::open()
{
	if(!isOpen)
		toggleOpen();
}

void Door::updateUvs(float uvY0, float uvY1)
{
	float uvs[8] = {
		0.f, uvY0,
		1.f, uvY0,
		1.f, uvY1,
		0.f, uvY1
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	for(int i=0; i<4; ++i)
		glBufferSubData(GL_ARRAY_BUFFER, (i * 4 + 2) * sizeof(float), 2 * sizeof(float), &uvs[i * 2]);
}
