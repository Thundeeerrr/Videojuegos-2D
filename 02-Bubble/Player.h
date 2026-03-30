#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// Player is basically a Sprite that represents the player. As such it has
// all properties it needs to track its movement, jumping, and collisions.


class Player
{
public:
	Player();
	~Player();

	enum class DoorState { NONE, ENTERING, ENTERED };

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();
	
	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	glm::vec2 getPosition() const;
	bool isDoorInteractionStarted() const;
	bool hasDoorTransitionEnded() const;
	void resetDoorState();
	
private:
	bool bJumping;
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY;
	Texture spritesheet, healthTexture;
	Sprite *sprite, *healthSprite;
	TileMap *map;
	int health;
  DoorState doorState;
	int doorTimer;
};


#endif // _PLAYER_INCLUDE


