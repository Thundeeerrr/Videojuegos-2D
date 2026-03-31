#include "PiolinEnemy.h"

const char* PiolinEnemy::getTextureFile() const
{
	return "images/Piolin-Sprites.png";
}

glm::ivec2 PiolinEnemy::getFrameSizePx() const
{
	return glm::ivec2(12, 8);
}

void PiolinEnemy::configureAnimations(float texW, float texH)
{
	auto uv = [&](int col, int row) -> glm::vec2
	{
		const float u = (float(col) * float(getFrameSizePx().x)) / texW;
		const float v = (float(row) * float(getFrameSizePx().y)) / texH;
		return glm::vec2(u, v);
	};

	sprite->setAnimationSpeed(STAND, 10);
	sprite->addKeyframe(STAND, uv(0, 0));

	sprite->setAnimationSpeed(WALK, 12);
	sprite->addKeyframe(WALK, uv(1, 0));
	sprite->addKeyframe(WALK, uv(2, 0));
	sprite->addKeyframe(WALK, uv(3, 0));
}

void PiolinEnemy::stepAI(int, const glm::ivec2 &bugsTilePos)
{
	if(canSeeBugs(bugsTilePos))
	{
		setState(State::CHASE);
		//moveTowardTile(bugsTilePos);
	}
	else
	{
		setState(State::PATROL);
		patrolStep();
	}
}