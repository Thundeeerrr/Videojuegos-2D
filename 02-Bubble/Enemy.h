#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE

#include <glm/glm.hpp>
#include "Sprite.h"
#include "TileMap.h"

class Enemy
{
public:
	enum class Type { DONALD, PIOLIN };
	enum class State { PATROL, CHASE, SEARCH };

public:
	Enemy();
	~Enemy();

	void init(Type type, const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime, const glm::vec2 &bugsWorldPos);
	void render();

	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	glm::vec2 getPosition() const;

	State getState() const { return state; }
	Type getType() const { return enemyType; }

private:
	bool canSeeBugs(const glm::ivec2 &bugsTilePos) const;
	void choosePatrolDirection();
	void stepAI(int deltaTime, const glm::ivec2 &bugsTilePos);
	bool moveHorizontal(int dir, int stepPx);
	void moveVertical(int dir, int stepPx);

	glm::ivec2 worldToTileCenterFoot(const glm::vec2 &worldPos) const;

private:
	Type enemyType;
	State state;

	glm::ivec2 tileMapDispl;
	glm::ivec2 posEnemy;

	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;

	bool facingRight;
	glm::ivec2 patrolDir;

	int searchTimerMs;
	glm::ivec2 lastSeenBugsTile;

	// Collision/visual constants (kept consistent with Player: 16x16 collision footprint)
	static const int COLLISION_W_PX = 16;
	static const int COLLISION_H_PX = 16;

	// How fast enemies move (px/frame step style, like Player uses 2px)
	// 2 px per update keeps movement consistent with Player::update().
	static const int MOVE_STEP_PX = 1;
	static const int FALL_STEP_PX = 2;
	// How far (in tiles) an enemy is allowed to "see" along a straight line.
	// This avoids enemies tracking across the entire map through corridors.
	static const int VISION_RANGE_TILES = 10;

	// How long enemy stays in SEARCH after losing line-of-sight.
	static const int SEARCH_DURATION_MS = 1200;

	static const int SPRITE_W_PX = 16;
	static const int SPRITE_H_PX = 24;

	// Enemy speed in pixels per second (tune this)
	static const float MOVE_SPEED_PX_PER_SEC;

	// Actual frame size derived from spritesheet
	int frameWidthPx;
	int frameHeightPx;
};

#endif // _ENEMY_INCLUDE