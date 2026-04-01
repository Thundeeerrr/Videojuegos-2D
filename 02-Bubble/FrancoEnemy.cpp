#include <cmath>
#include "FrancoEnemy.h"

const char* FrancoEnemy::getTextureFile() const
{
 return "images/FrancoEnemy.png";
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

 sprite->setAnimationSpeed(WALK, 12);
	sprite->addKeyframe(WALK, uv(0, 0));
	sprite->addKeyframe(WALK, uv(1, 0));
	sprite->addKeyframe(WALK, uv(2, 0));

	sprite->setAnimationSpeed(STAND_LEFT, 7);
	sprite->addKeyframe(STAND_LEFT, uv(0, 1));

	sprite->setAnimationSpeed(WALK_LEFT, 12);
	sprite->addKeyframe(WALK_LEFT, uv(0, 1));
	sprite->addKeyframe(WALK_LEFT, uv(1, 1));
	sprite->addKeyframe(WALK_LEFT, uv(2, 1));

	sprite->setAnimationSpeed(SHOOT, SHOOT_FPS);
	sprite->addKeyframe(SHOOT, uv(0, 2));
	sprite->addKeyframe(SHOOT, uv(1, 2));
	sprite->addKeyframe(SHOOT, uv(2, 2));
	sprite->addKeyframe(SHOOT, uv(3, 2));

	sprite->setAnimationSpeed(SHOOT_LEFT, SHOOT_FPS);
	sprite->addKeyframe(SHOOT_LEFT, uv(0, 3));
	sprite->addKeyframe(SHOOT_LEFT, uv(1, 3));
	sprite->addKeyframe(SHOOT_LEFT, uv(2, 3));
	sprite->addKeyframe(SHOOT_LEFT, uv(3, 3));
}

bool FrancoEnemy::getForcedAnimationId(int &animId) const
{
	if(shootAnimRemainingMs > 0)
	{
		animId = facingRight ? SHOOT : SHOOT_LEFT;
		return true;
	}

	return false;
}

bool FrancoEnemy::consumePendingShot(int &dirSign)
{
	if(pendingShotDir == 0)
		return false;

	dirSign = pendingShotDir;
	pendingShotDir = 0;
	return true;
}

void FrancoEnemy::stepAI(int deltaTime, const glm::ivec2 &bugsTilePos)
{
    if(shootAnimRemainingMs > 0)
	{
		shootAnimRemainingMs -= deltaTime;
		if(shootAnimRemainingMs < 0)
			shootAnimRemainingMs = 0;
		return;
	}

	shootCooldownMs -= deltaTime;
	if(shootCooldownMs <= 0)
	{
		shootCooldownMs = SHOOT_INTERVAL_MS;
		shootAnimRemainingMs = SHOOT_ANIM_DURATION_MS;
     pendingShotDir = facingRight ? 1 : -1;
		return;
	}

  const glm::ivec2 myTile = getMyTile();
	const int dx = bugsTilePos.x - myTile.x;
	const int dy = bugsTilePos.y - myTile.y;

	auto moveTowardTileHorizontal = [&](const glm::ivec2 &targetTile)
	{
		const int localDx = targetTile.x - getMyTile().x;
		if(localDx > 0)
			moveHorizontal(1, MOVE_STEP_PX);
		else if(localDx < 0)
			moveHorizontal(-1, MOVE_STEP_PX);
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

void FrancoEnemy::enforceHorizontalPatrolRange(float minX, float maxX)
{
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
			map->collisionMoveDown(glm::ivec2(int(std::round(pos.x)), correctedY), glm::ivec2(COLLISION_W_PX, COLLISION_H_PX), &correctedY);
		const int oneTilePx = (map != NULL) ? map->getTileSize() : COLLISION_H_PX;
		patrolBaseY = correctedY - oneTilePx;
	}

	pos.y = float(patrolBaseY);
	setPosition(pos);
}
