#include <cmath>
#include "DonaldEnemy.h"
#include <iostream>
const char* DonaldEnemy::getTextureFile() const
{
	return "images/PatoDonald-Sprites.png";
}

glm::ivec2 DonaldEnemy::getFrameSizePx() const
{
	return glm::ivec2(16, 32);
}

void DonaldEnemy::configureAnimations(float texW, float texH)
{
	auto uv = [&](int col, int row) -> glm::vec2
	{
		const float u = (float(col) * float(getFrameSizePx().x)) / texW;
		const float v = (float(row) * float(getFrameSizePx().y)) / texH;
		return glm::vec2(u, v);
	};

	sprite->setAnimationSpeed(STAND, 8);
	sprite->addKeyframe(STAND, uv(0, 0));

	sprite->setAnimationSpeed(WALK, 8);
	sprite->addKeyframe(WALK, uv(1, 0));
	sprite->addKeyframe(WALK, uv(2, 0));
	sprite->addKeyframe(WALK, uv(3, 0));
	sprite->addKeyframe(WALK, uv(4, 0));
}

void DonaldEnemy::moveTowardTile(const glm::ivec2& bugsTilePos, int stepPx)
{
	const glm::ivec2 myTile = getMyTile();
	const int dx = bugsTilePos.x - myTile.x;
	const int dy = bugsTilePos.y - myTile.y;

	const glm::ivec2 pos(int(getPosition().x), int(getPosition().y));
	const bool onVine = map->isStairTileForBody(pos, glm::ivec2(COLLISION_W_PX, COLLISION_H_PX));

	auto sgn = [](int v) -> int { return (v < 0) ? -1 : ((v > 0) ? 1 : 0); };

	if(std::abs(dy) > std::abs(dx))
	{
		if(onVine)
		{
			// Align enemy center to current vine column center before vertical movement.
			const int tileSize = map->getTileSize();
			const int enemyCenterX = pos.x + (COLLISION_W_PX / 2);
			const int vineTileX = enemyCenterX / tileSize;
			const int vineCenterX = vineTileX * tileSize + (tileSize / 2);
			const int targetPosX = vineCenterX - (COLLISION_W_PX / 2);
			const int alignDx = targetPosX - pos.x;

			if(alignDx != 0)
				moveHorizontal(sgn(alignDx), stepPx);
			else
				moveVertical(sgn(dy), stepPx);
		}
		else if(dx != 0)
		{
			moveHorizontal(sgn(dx), stepPx);
		}
		return;
	}

	if(dx != 0)
		moveHorizontal(sgn(dx), stepPx);
}

void DonaldEnemy::stepAI(int deltaTime, const glm::ivec2 &bugsTilePos)
{
	switch(getState())
	{
		case State::PATROL:
		{
			if(canSeeBugs(bugsTilePos))
			{
				setState(State::CHASE); // important
				setLastSeenBugsTile(bugsTilePos);
				setSearchTimerMs(0);
				moveTowardTile(bugsTilePos);
				break;
			}
			patrolStep();
			break;
		}

		case State::CHASE:
		{
			if(canSeeBugs(bugsTilePos))
			{
				setLastSeenBugsTile(bugsTilePos);
				setSearchTimerMs(0);
				moveTowardTile(bugsTilePos);
			}
			else
			{
				setState(State::SEARCH);
				setSearchTimerMs(getSearchDurationMs());
			}
			break;
		}

		case State::SEARCH:
		{
			if(canSeeBugs(bugsTilePos))
			{
				setState(State::CHASE);
				setLastSeenBugsTile(bugsTilePos);
				setSearchTimerMs(0);
				moveTowardTile(bugsTilePos);
				break;
			}

			setSearchTimerMs(getSearchTimerMs() - deltaTime);
			if(getSearchTimerMs() <= 0)
			{
				setState(State::PATROL);
				pickPatrolDirection();
				break;
			}

			moveTowardTile(getLastSeenBugsTile());
			break;
		}
	}
}