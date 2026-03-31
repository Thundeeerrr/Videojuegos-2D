#include <cstdlib>
#include <cmath>
#include "Enemy.h"
#include <iostream>
#include "Game.h"
#include <glm/gtc/matrix_transform.hpp>

const float Enemy::MOVE_SPEED_PX_PER_SEC = 48.f;
namespace
{
	// Sprite sheet / animation indices are placeholders.
	// Keep them here so you can later map them to your actual enemy spritesheet.
	enum EnemyAnims
	{
		STAND, WALK
	};

	// Convert a tile delta to a cardinal direction vector (-1/0/1).
	int signInt(int v)
	{
		if(v < 0) return -1;
		if(v > 0) return 1;
		return 0;
	}
}

Enemy::Enemy()
{
	state = State::PATROL;
	sprite = NULL;
	map = NULL;
	facingRight = true;
	patrolDir = glm::ivec2(1, 0);
	searchTimerMs = 0;
	lastSeenBugsTile = glm::ivec2(0);
}

Enemy::~Enemy()
{
	if(sprite != NULL)
		delete sprite;
}

void Enemy::init(Type type, const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	enemyType = type;
	state = State::PATROL;
	tileMapDispl = tileMapPos;
	searchTimerMs = 0;

	string textureFile;
	int sheetCols = 0;
	int sheetRows = 0;

	if(type == Type::DONALD)
	{
		textureFile = "images/PatoDonald-Sprites.png";
		frameWidthPx = 16;
		frameHeightPx = 32;
	}
	else
	{
		textureFile = "images/Piolin-Sprites.png";
		frameWidthPx = 12;
		frameHeightPx = 8;
	}

	spritesheet.loadFromFile(textureFile, TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheet.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);

	const float texW = float(spritesheet.width());
	const float texH = float(spritesheet.height());

	const float uvSizeX = float(frameWidthPx) / texW;
	const float uvSizeY = float(frameHeightPx) / texH;

	sprite = Sprite::createSprite(
		glm::ivec2(frameWidthPx, frameHeightPx),
		glm::vec2(uvSizeX, uvSizeY),
		&spritesheet,
		&shaderProgram);

	sprite->setNumberAnimations(2);

	auto uv = [&](int col, int row) -> glm::vec2
	{
		const float u = (float(col) * float(frameWidthPx)) / texW;
		const float v = (float(row) * float(frameHeightPx)) / texH;
		return glm::vec2(u, v);
	};

	sprite->setAnimationSpeed(STAND, 8);
	sprite->addKeyframe(STAND, uv(0, 0));

	sprite->setAnimationSpeed(WALK, 8);
	sprite->addKeyframe(WALK, uv(1, 0));
	sprite->addKeyframe(WALK, uv(2, 0));
	sprite->addKeyframe(WALK, uv(3, 0));
	sprite->addKeyframe(WALK, uv(4, 0));

	sprite->changeAnimation(STAND);

	choosePatrolDirection();
	setPosition(glm::vec2(0.f, 0.f));
}

void Enemy::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Enemy::setPosition(const glm::vec2 &pos)
{
	posEnemy = glm::ivec2(int(pos.x), int(pos.y));
	if(sprite != NULL)
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

glm::vec2 Enemy::getPosition() const
{
	return glm::vec2(float(posEnemy.x), float(posEnemy.y));
}

void Enemy::render()
{
	if(sprite == NULL)
		return;

	if(!facingRight)
	{
		glm::mat4 local = glm::translate(glm::mat4(1.0f), glm::vec3(float(frameWidthPx), 0.f, 0.f));
		local = glm::scale(local, glm::vec3(-1.f, 1.f, 1.f));
		sprite->setLocalTransform(local);
	}
	else
	{
		sprite->setLocalTransform(glm::mat4(1.0f));
	}

	sprite->render();
}

glm::ivec2 Enemy::worldToTileCenterFoot(const glm::vec2 &worldPos) const
{
	// Uses the same probe idea as Scene uses for player: (x+8, y+15) for a 16x16 collider.
	return map->worldToTile(worldPos + glm::vec2(8.f, 15.f));
}

bool Enemy::canSeeBugs(const glm::ivec2 &bugsTilePos) const
{
	if(map == NULL)
		return false;

	glm::ivec2 myTile = worldToTileCenterFoot(getPosition());

	const int dx = bugsTilePos.x - myTile.x;
	const int dy = bugsTilePos.y - myTile.y;

	// Only straight-line detection (same row or same column)
	if(dx != 0 && dy != 0)
		return false;

	const int dist = std::abs(dx) + std::abs(dy);
	if(dist > VISION_RANGE_TILES)
		return false;

	// No wall-blocking yet (since isSolidTile() doesn't exist).
	// This can be upgraded later to stop vision through solid tiles.
	return true;
}

void Enemy::choosePatrolDirection()
{
	if(map == NULL)
	{
		patrolDir = glm::ivec2(1, 0);
		return;
	}

	const int maxX = int(map->getMapSize().x) * map->getTileSize() - COLLISION_W_PX;

	if(posEnemy.x <= 0)
		patrolDir = glm::ivec2(1, 0);
	else if(posEnemy.x >= maxX)
		patrolDir = glm::ivec2(-1, 0);
	else
		patrolDir = glm::ivec2((std::rand() % 2 == 0) ? 1 : -1, 0);
}

bool Enemy::moveHorizontal(int dir, int stepPx)
{
	if(dir == 0 || map == NULL || stepPx <= 0)
		return true;

	facingRight = (dir > 0);

	const int minX = 0;
	const int maxX = int(map->getMapSize().x) * map->getTileSize() - COLLISION_W_PX;

	const int prevX = posEnemy.x;
	posEnemy.x += dir * stepPx;

	bool blocked = false;

	if(posEnemy.x < minX)
	{
		posEnemy.x = minX;
		blocked = true;
	}
	else if(posEnemy.x > maxX)
	{
		posEnemy.x = maxX;
		blocked = true;
	}

	// If you later add real tile collision, keep this:
	if(!blocked)
	{
		if(dir < 0 && map->collisionMoveLeft(posEnemy, glm::ivec2(COLLISION_W_PX, COLLISION_H_PX)))
		{
			posEnemy.x = prevX;
			blocked = true;
		}
		else if(dir > 0 && map->collisionMoveRight(posEnemy, glm::ivec2(COLLISION_W_PX, COLLISION_H_PX)))
		{
			posEnemy.x = prevX;
			blocked = true;
		}
	}

	return blocked;
}

void Enemy::moveVertical(int dir, int stepPx)
{
	if(dir == 0 || map == NULL || stepPx <= 0)
		return;

	posEnemy.y += dir * stepPx;

	int correctedY = posEnemy.y;
	if(map->collisionMoveDown(posEnemy, glm::ivec2(COLLISION_W_PX, COLLISION_H_PX), &correctedY))
		posEnemy.y = correctedY;
}

void Enemy::stepAI(int deltaTime, const glm::ivec2 &bugsTilePos)
{
	const glm::ivec2 myTile = worldToTileCenterFoot(getPosition());
	const int stepPx = MOVE_STEP_PX;

	switch(state)
	{
		case State::PATROL:
		{
			if(patrolDir.x != 0)
			{
				const bool blocked = moveHorizontal(patrolDir.x, stepPx);
				if(blocked)
					patrolDir.x = -patrolDir.x;
			}
			else
			{
				const int prevY = posEnemy.y;
				moveVertical(patrolDir.y, stepPx);
				if(posEnemy.y == prevY)
					choosePatrolDirection();
			}
			break;
		}

		case State::CHASE:
		{
			if(canSeeBugs(bugsTilePos))
			{
				lastSeenBugsTile = bugsTilePos;
				searchTimerMs = 0;
			}
			else
			{
				state = State::SEARCH;
				searchTimerMs = SEARCH_DURATION_MS;
			}

			const int dx = bugsTilePos.x - myTile.x;
			const int dy = bugsTilePos.y - myTile.y;

			if(std::abs(dx) >= std::abs(dy)) moveHorizontal(signInt(dx), stepPx);
			else                             moveVertical(signInt(dy), stepPx);

			break;
		}

		case State::SEARCH:
		{
			if(canSeeBugs(bugsTilePos))
			{
				state = State::CHASE;
				lastSeenBugsTile = bugsTilePos;
				searchTimerMs = 0;
				break;
			}

			searchTimerMs -= deltaTime;
			if(searchTimerMs <= 0)
			{
				state = State::PATROL;
				choosePatrolDirection();
				break;
			}

			const int dx = lastSeenBugsTile.x - myTile.x;
			const int dy = lastSeenBugsTile.y - myTile.y;

			if(std::abs(dx) >= std::abs(dy)) moveHorizontal(signInt(dx), stepPx);
			else                             moveVertical(signInt(dy), stepPx);

			break;
		}
	}
}

void Enemy::update(int deltaTime, const glm::vec2 &bugsWorldPos)
{
	if(sprite == NULL || map == NULL)
		return;

	sprite->update(deltaTime);

	// Gravity (time-based)
	{
		posEnemy.y += FALL_STEP_PX;
		map->collisionMoveDown(posEnemy, glm::ivec2(COLLISION_W_PX, COLLISION_H_PX), &posEnemy.y);
	}

	const glm::ivec2 bugsTile = worldToTileCenterFoot(bugsWorldPos);

	const glm::ivec2 prevPos = posEnemy;
	stepAI(deltaTime, bugsTile);
	const bool moved = (posEnemy != prevPos);

	if(moved)
	{
		//cout << "[EnemyDebug] Enemy at tile (" << bugsTile.x << "," << bugsTile.y << ") state=" << int(state) << " moved to (" << posEnemy.x << "," << posEnemy.y << ")" << endl;
		if(sprite->animation() != WALK)
			sprite->changeAnimation(WALK);
	}
	else
	{
		cout << "[EnemyDebug] Enemy at tile (" << bugsTile.x << "," << bugsTile.y << ") state=" << int(state) << " searchTimer=" << searchTimerMs << endl;
		if(sprite->animation() != STAND)
			sprite->changeAnimation(STAND);
	}

	// Visual offset so 24px sprite sits on 16px collider feet
	const int visualOffsetY = COLLISION_H_PX - frameHeightPx;
	sprite->setPosition(glm::vec2(
		float(tileMapDispl.x + posEnemy.x),
		float(tileMapDispl.y + posEnemy.y + visualOffsetY)));
}