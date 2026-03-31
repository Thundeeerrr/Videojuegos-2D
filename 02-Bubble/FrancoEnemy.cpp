#include "FrancoEnemy.h"

const char* FrancoEnemy::getTextureFile() const
{
	// Replace with real Franco texture when available.
	return "images/PatoDonald-Sprites.png";
}

glm::ivec2 FrancoEnemy::getFrameSizePx() const
{
	return glm::ivec2(16, 32);
}

void FrancoEnemy::configureAnimations(float texW, float texH)
{
	auto uv = [&](int col, int row) -> glm::vec2
	{
		const float u = (float(col) * float(getFrameSizePx().x)) / texW;
		const float v = (float(row) * float(getFrameSizePx().y)) / texH;
		return glm::vec2(u, v);
	};

	sprite->setAnimationSpeed(STAND, 7);
	sprite->addKeyframe(STAND, uv(0, 0));

	sprite->setAnimationSpeed(WALK, 9);
	sprite->addKeyframe(WALK, uv(1, 0));
	sprite->addKeyframe(WALK, uv(2, 0));
	sprite->addKeyframe(WALK, uv(3, 0));
	sprite->addKeyframe(WALK, uv(4, 0));
}

void FrancoEnemy::stepAI(int deltaTime, const glm::ivec2 &bugsTilePos)
{
	switch(getState())
	{
		case State::PATROL:
			if(canSeeBugs(bugsTilePos))
			{
				setState(State::CHASE);
				setLastSeenBugsTile(bugsTilePos);
			}
			else
				patrolStep();
			break;

		case State::CHASE:
			if(canSeeBugs(bugsTilePos))
			{
				setLastSeenBugsTile(bugsTilePos);
				setSearchTimerMs(getSearchDurationMs());
				//moveTowardTile(bugsTilePos);
			}
			else
				setState(State::SEARCH);
			break;

		case State::SEARCH:
			setSearchTimerMs(getSearchTimerMs() - deltaTime);
			if(getSearchTimerMs() <= 0)
			{
				setState(State::PATROL);
				pickPatrolDirection();
			}
			else
				//moveTowardTile(getLastSeenBugsTile());
			break;
	}
}