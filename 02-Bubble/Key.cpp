#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Key.h"

namespace
{
	const int KEY_TILE_SIZE = 16;
	const int KEY_TILE_ID = 85;
	const int KEY_TILESET_COLS = 16;
	const int KEY_TILESET_ROWS = 9;
}

Key::Key()
{
	worldPos = glm::vec2(0.f);
	tilePos = glm::ivec2(0);
	collected = false;
	vao = 0;
	vbo = 0;
	posLocation = -1;
	texCoordLocation = -1;
	shaderProgram = NULL;
}

Key::~Key()
{
	if(vao != 0)
		glDeleteVertexArrays(1, &vao);
	if(vbo != 0)
		glDeleteBuffers(1, &vbo);
}

void Key::init(const glm::ivec2& tilePos, ShaderProgram& shaderProgram)
{
	this->tilePos = tilePos;
	this->shaderProgram = &shaderProgram;
	collected = false;

	worldPos.x = float(tilePos.x * KEY_TILE_SIZE);
	worldPos.y = float(tilePos.y * KEY_TILE_SIZE);

	keyTexture.loadFromFile("images/key.png", TEXTURE_PIXEL_FORMAT_RGBA);
	keyTexture.setWrapS(GL_CLAMP_TO_EDGE);
	keyTexture.setWrapT(GL_CLAMP_TO_EDGE);
	keyTexture.setMinFilter(GL_NEAREST);
	keyTexture.setMagFilter(GL_NEAREST);

	// Full texture UVs: key.png is not an atlas
	const float uvX0 = 0.f, uvY0 = 0.f;
	const float uvX1 = 1.f, uvY1 = 1.f;

	float vertices[16] = {
		0.f, 0.f, uvX0, uvY0,
		float(KEY_TILE_SIZE), 0.f, uvX1, uvY0,
		float(KEY_TILE_SIZE), float(KEY_TILE_SIZE), uvX1, uvY1,
		0.f, float(KEY_TILE_SIZE), uvX0, uvY1
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	posLocation = shaderProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	texCoordLocation = shaderProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void Key::update(int deltaTime)
{
}

void Key::render() const
{
	if(collected)
		return;

	glm::mat4 modelview = glm::translate(glm::mat4(1.0f), glm::vec3(worldPos.x, worldPos.y, 0.f));
	shaderProgram->setUniformMatrix4f("modelview", modelview);
	shaderProgram->setUniform2f("texCoordDispl", 0.f, 0.f);
	glEnable(GL_TEXTURE_2D);
	keyTexture.use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisable(GL_TEXTURE_2D);
}

void Key::collect()
{
	collected = true;
}
