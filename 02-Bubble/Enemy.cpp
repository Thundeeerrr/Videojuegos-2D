#include <cstdlib>
#include <cmath>
#include <iostream>
#include "Enemy.h"
#include "Game.h"
#include <glm/gtc/matrix_transform.hpp>

bool Enemy::freezed = false;
namespace
{
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
	frameWidthPx = 16;
	frameHeightPx = 16;
}

Enemy::~Enemy()
{
	if(sprite != NULL)
		delete sprite;
}

void Enemy::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	state = State::PATROL;
	tileMapDispl = tileMapPos;
	searchTimerMs = 0;

	const glm::ivec2 frameSize = getFrameSizePx();
	frameWidthPx = frameSize.x;
	frameHeightPx = frameSize.y;

	spritesheet.loadFromFile(getTextureFile(), TEXTURE_PIXEL_FORMAT_RGBA);
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

 sprite->setNumberAnimations(getAnimationCount());
	configureAnimations(texW, texH);
 sprite->changeAnimation(getStandAnimationId());

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
 {
		const int visualOffsetY = COLLISION_H_PX - frameHeightPx;
		sprite->setPosition(glm::vec2(
			float(tileMapDispl.x + posEnemy.x),
			float(tileMapDispl.y + posEnemy.y + visualOffsetY)));
	}
}

glm::vec2 Enemy::getPosition() const
{
	return glm::vec2(float(posEnemy.x), float(posEnemy.y));
}

void Enemy::render()
{
	if(sprite == NULL)
		return;

    if(!facingRight && shouldMirrorWhenFacingLeft())
	{
		glm::mat4 local = glm::translate(glm::mat4(1.0f), glm::vec3(float(frameWidthPx), 0.f, 0.f));
		local = glm::scale(local, glm::vec3(-1.f, 1.f, 1.f));
		sprite->setLocalTransform(local);
	}
	else
		sprite->setLocalTransform(glm::mat4(1.0f));

	sprite->render();
}

glm::ivec2 Enemy::worldToTileCenterFoot(const glm::vec2 &worldPos) const
{
	return map->worldToTile(worldPos + glm::vec2(8.f, 15.f));
}

glm::ivec2 Enemy::getMyTile() const
{
	return worldToTileCenterFoot(getPosition());
}

bool Enemy::canSeeBugs(const glm::ivec2 &bugsTilePos) const
{
	if(map == NULL)
		return false;

	const glm::ivec2 myTile = getMyTile();
	const int dx = bugsTilePos.x - myTile.x;
	const int dy = bugsTilePos.y - myTile.y;

	if(dx != 0 && dy != 0)
		return false;

	const int dist = std::abs(dx) + std::abs(dy);
	if(dist > getVisionRangeTiles())
		return false;

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
	if(posEnemy.x <= 0) patrolDir = glm::ivec2(1, 0);
	else if(posEnemy.x >= maxX) patrolDir = glm::ivec2(-1, 0);
	else patrolDir = glm::ivec2((std::rand() % 2 == 0) ? 1 : -1, 0);
}

void Enemy::pickPatrolDirection()
{
	choosePatrolDirection();
}

void Enemy::patrolStep(int stepPx)
{
	if(patrolDir.x != 0)
	{
		const bool blocked = moveHorizontal(patrolDir.x, stepPx);
		if(blocked) patrolDir.x = -patrolDir.x;
	}
	else
	{
		const int prevY = posEnemy.y;
		moveVertical(patrolDir.y, stepPx);
		if(posEnemy.y == prevY) choosePatrolDirection();
	}
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
	if(posEnemy.x < minX) { posEnemy.x = minX; blocked = true; }
	else if(posEnemy.x > maxX) { posEnemy.x = maxX; blocked = true; }

	if(!blocked)
	{
		if(dir < 0 && map->collisionMoveLeft(posEnemy, glm::ivec2(COLLISION_W_PX, COLLISION_H_PX)))
		{
			posEnemy.x = prevX; blocked = true;
		}
		else if(dir > 0 && map->collisionMoveRight(posEnemy, glm::ivec2(COLLISION_W_PX, COLLISION_H_PX)))
		{
			posEnemy.x = prevX; blocked = true;
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

	if(dir < 0)
	{
		// Moving up -> use ceiling collision
		if(map->collisionMoveUp(posEnemy, glm::ivec2(COLLISION_W_PX, COLLISION_H_PX), &correctedY))
			posEnemy.y = correctedY;
	}
	else
	{
		// Moving down -> use floor collision
		if(map->collisionMoveDown(posEnemy, glm::ivec2(COLLISION_W_PX, COLLISION_H_PX), &correctedY))
			posEnemy.y = correctedY;
	}
}

void Enemy::update(int deltaTime, const glm::vec2 &bugsWorldPos)
{
	if(sprite == NULL || map == NULL)
		return;
	sprite->update(deltaTime);
	if (freezed)	return;
	const bool onClimbable = map->isStairTileForBody(
		posEnemy,
		glm::ivec2(COLLISION_W_PX, COLLISION_H_PX));

	// Apply gravity only when not on vine/stair.
	// Reasoning: climbing step is MOVE_STEP_PX=1 while gravity is FALL_STEP_PX=2,
	// so gravity must be gated or upward movement is canceled.
	if(!onClimbable)
	{
		const int footProbeMarginPx = COLLISION_W_PX / 4;
		const glm::ivec2 feetProbeSize(COLLISION_W_PX - (2 * footProbeMarginPx), COLLISION_H_PX);

		posEnemy.y += FALL_STEP_PX;

		glm::ivec2 probePos = posEnemy;
		probePos.x += footProbeMarginPx;

		int correctedY = posEnemy.y;
		if(map->collisionMoveDown(probePos, feetProbeSize, &correctedY))
			posEnemy.y = correctedY;
	}

	const glm::ivec2 bugsTile = worldToTileCenterFoot(bugsWorldPos);
	const glm::ivec2 prevPos = posEnemy;

	stepAI(deltaTime, bugsTile);

    int forcedAnim = -1;
	if(getForcedAnimationId(forcedAnim))
	{
		if(sprite->animation() != forcedAnim)
			sprite->changeAnimation(forcedAnim);
	}
	else
	{
		const bool moved = (posEnemy != prevPos);
		const int standAnim = getStandAnimationId();
		const int walkAnim = getWalkAnimationId();
		if(moved)
		{
			if(sprite->animation() != walkAnim)
				sprite->changeAnimation(walkAnim);
		}
		else
		{
			if(sprite->animation() != standAnim)
				sprite->changeAnimation(standAnim);
		}
	}

	const int visualOffsetY = COLLISION_H_PX - frameHeightPx;
	sprite->setPosition(glm::vec2(
		float(tileMapDispl.x + posEnemy.x),
		float(tileMapDispl.y + posEnemy.y + visualOffsetY)));
}

void Enemy::Freeze()
{
	freezed = true;
}

void Enemy::Unfreeze()
{
	freezed = false;
}