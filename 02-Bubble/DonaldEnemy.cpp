#include <cmath>
#include "DonaldEnemy.h"

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

	auto sgn = [](int v) -> int { return (v < 0) ? -1 : ((v > 0) ? 1 : 0); };

	const glm::ivec2 bodySize(COLLISION_W_PX, COLLISION_H_PX);
	const glm::ivec2 posBefore(int(getPosition().x), int(getPosition().y));
	const bool onVine = map->isStairTileForBody(posBefore, bodySize);

	// If we are on vine and there is vertical gap, use it.
	if(onVine && dy != 0)
	{
		moveVertical(sgn(dy), stepPx);

		const glm::ivec2 posAfter(int(getPosition().x), int(getPosition().y));
		const bool verticalMoved = (posAfter.y != posBefore.y);

		// If blocked at vine top/bottom, leave vine horizontally.
		if(!verticalMoved)
			moveHorizontal(facingRight ? 1 : -1, stepPx);

		return;
	}

	// Horizontal-first policy with anti-flip dead-zone:
	// while vertical gap exists, avoid immediate turnaround near aligned X.
	if(dy != 0)
	{
		int dirX;
		if(std::abs(dx) > 1) dirX = sgn(dx);
		else                 dirX = facingRight ? 1 : -1;

		moveHorizontal(dirX, stepPx);
		return;
	}

	// Same height: normal horizontal chase.
	if(dx != 0)
		moveHorizontal(sgn(dx), stepPx);
}

void DonaldEnemy::stepAI(int deltaTime, const glm::ivec2 &bugsTilePos)
{
	const glm::ivec2 myTile = getMyTile();
	const int dx = bugsTilePos.x - myTile.x;
	const int dy = bugsTilePos.y - myTile.y;
	const int manhattanDist = std::abs(dx) + std::abs(dy);

	// Distance-based pursuit to keep behavior stable.
	// Multiplier 2 keeps Donald reactive without global-map tracking.
	const int pursuitRangeTiles = getVisionRangeTiles() * 2;
	const bool shouldChase = (manhattanDist <= pursuitRangeTiles);

	switch(getState())
	{
		case State::PATROL:
		{
			if(shouldChase)
			{
				setState(State::CHASE);
				setLastSeenBugsTile(bugsTilePos);
				setSearchTimerMs(0);
				moveTowardTile(bugsTilePos);
			}
			else
				patrolStep();
			break;
		}

		case State::CHASE:
		{
			if(shouldChase)
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
			if(shouldChase)
			{
				setState(State::CHASE);
				setLastSeenBugsTile(bugsTilePos);
				setSearchTimerMs(0);
				moveTowardTile(bugsTilePos);
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
					moveTowardTile(getLastSeenBugsTile());
			}
			break;
		}
	}
}