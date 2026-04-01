#include <cmath>
#include "PiolinEnemy.h"

const char* PiolinEnemy::getTextureFile() const
{
	return "images/Piolin-Sprites.png";
}

glm::ivec2 PiolinEnemy::getFrameSizePx() const
{
   return glm::ivec2(16, 32);
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
    sprite->addKeyframe(WALK, uv(0, 0));
	sprite->addKeyframe(WALK, uv(1, 0));
	sprite->addKeyframe(WALK, uv(2, 0));
}

void PiolinEnemy::stepAI(int deltaTime, const glm::ivec2 &bugsTilePos)
{
 const glm::ivec2 myTile = getMyTile();
	const int dx = bugsTilePos.x - myTile.x;
	const int dy = bugsTilePos.y - myTile.y;

	auto moveTowardTileHorizontal = [&](const glm::ivec2 &targetTile)
	{
		const int localDx = targetTile.x - getMyTile().x;
     if(localDx > 0)
		{
			if(hasHorizontalRange && getPosition().x >= rangeMaxX)
				moveHorizontal(-1, MOVE_STEP_PX);
			else
				moveHorizontal(1, MOVE_STEP_PX);
		}
		else if(localDx < 0)
		{
			if(hasHorizontalRange && getPosition().x <= rangeMinX)
				moveHorizontal(1, MOVE_STEP_PX);
			else
				moveHorizontal(-1, MOVE_STEP_PX);
		}
	};

	const bool canChaseHorizontally = (dy == 0 && std::abs(dx) <= getVisionRangeTiles());

	switch(getState())
	{
     case State::PATROL:
		{
			if(canChaseHorizontally)
			{
				setState(State::CHASE);
				setLastSeenBugsTile(bugsTilePos);
				setSearchTimerMs(0);
				moveTowardTileHorizontal(bugsTilePos);
			}
			else
				patrolStep();
			break;
		}

		case State::CHASE:
		{
			if(canChaseHorizontally)
			{
				setLastSeenBugsTile(bugsTilePos);
				setSearchTimerMs(0);
				moveTowardTileHorizontal(bugsTilePos);
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
			if(canChaseHorizontally)
			{
				setState(State::CHASE);
				setLastSeenBugsTile(bugsTilePos);
				setSearchTimerMs(0);
				moveTowardTileHorizontal(bugsTilePos);
			}
			else
			{
				setSearchTimerMs(getSearchTimerMs() - deltaTime);
				if(getSearchTimerMs() <= 0)
				{
					setState(State::PATROL);
					pickPatrolDirection();
				}
				else
					moveTowardTileHorizontal(getLastSeenBugsTile());
			}
			break;
		}
	}

}

void PiolinEnemy::enforceHorizontalPatrolRange(float minX, float maxX)
{
  hasHorizontalRange = true;
	rangeMinX = minX;
	rangeMaxX = maxX;

	glm::vec2 pos = getPosition();

	if(pos.x <= minX)
	{
		pos.x = minX;
		patrolDir = glm::ivec2(1, 0);
		facingRight = true;
	}
	else if(pos.x >= maxX)
	{
		pos.x = maxX;
		patrolDir = glm::ivec2(-1, 0);
		facingRight = false;
	}

	if(!hasPatrolBaseY)
	{
		hasPatrolBaseY = true;
        int correctedY = int(std::round(pos.y));
		if(map != NULL)
        {
			const int tileSize = map->getTileSize();
			int probeY = correctedY + tileSize;
			map->collisionMoveDown(glm::ivec2(int(std::round(pos.x)), probeY), glm::ivec2(COLLISION_W_PX, COLLISION_H_PX), &correctedY);
		}
       patrolBaseY = correctedY;
	}

	// Keep Piolin on his original patrol platform Y in constrained patrol maps.
	pos.y = float(patrolBaseY);
	setPosition(pos);
}
