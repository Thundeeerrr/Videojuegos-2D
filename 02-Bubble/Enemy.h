#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE

#include <glm/glm.hpp>
#include "Sprite.h"
#include "TileMap.h"


class Enemy
{
public:
	enum class Type { DONALD, PIOLIN, FRANCO };
	enum class State { PATROL, CHASE, SEARCH };

public:
	Enemy();
	virtual ~Enemy();

	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime, const glm::vec2 &bugsWorldPos);
	void render();

	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	glm::vec2 getPosition() const;
	glm::ivec2 getCollisionSize() const { return glm::ivec2(COLLISION_W_PX, COLLISION_H_PX); }

	State getState() const { return state; }
	virtual Type getType() const = 0;

protected:
	// Subclass responsibilities
	virtual const char* getTextureFile() const = 0;
	virtual glm::ivec2 getFrameSizePx() const = 0;
	virtual void configureAnimations(float texW, float texH) = 0;
	virtual void stepAI(int deltaTime, const glm::ivec2 &bugsTilePos) = 0;
	virtual int getVisionRangeTiles() const { return DEFAULT_VISION_RANGE_TILES; }
	virtual int getSearchDurationMs() const { return DEFAULT_SEARCH_DURATION_MS; }

	// Shared helpers for subclasses
	bool canSeeBugs(const glm::ivec2 &bugsTilePos) const;
	void patrolStep(int stepPx = MOVE_STEP_PX);
	bool moveHorizontal(int dir, int stepPx);
	void moveVertical(int dir, int stepPx);
	glm::ivec2 getMyTile() const;
	void pickPatrolDirection();

	void setState(State s) { state = s; }
	void setSearchTimerMs(int ms) { searchTimerMs = ms; }
	int getSearchTimerMs() const { return searchTimerMs; }
	void setLastSeenBugsTile(const glm::ivec2 &t) { lastSeenBugsTile = t; }
	glm::ivec2 getLastSeenBugsTile() const { return lastSeenBugsTile; }

	enum EnemyAnims { STAND = 0, WALK = 1 };

protected:
	static const int COLLISION_W_PX = 16;
	static const int COLLISION_H_PX = 16;
	static const int MOVE_STEP_PX = 1;
	static const int FALL_STEP_PX = 2;
	static const int DEFAULT_VISION_RANGE_TILES = 10;
	static const int DEFAULT_SEARCH_DURATION_MS = 1200;

	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;

	bool facingRight;
	glm::ivec2 patrolDir;

	int frameWidthPx;
	int frameHeightPx;

private:
	void choosePatrolDirection();
	glm::ivec2 worldToTileCenterFoot(const glm::vec2 &worldPos) const;

private:
	State state;
	glm::ivec2 tileMapDispl;
	glm::ivec2 posEnemy;

	int searchTimerMs;
	glm::ivec2 lastSeenBugsTile;
};

#endif // _ENEMY_INCLUDE