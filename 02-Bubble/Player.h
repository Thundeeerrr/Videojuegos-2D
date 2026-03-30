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

   enum class DoorState { NONE, ENTERING, ENTERED, LEAVING };
	enum class TubeState { NONE, ENTERING, TRAVELING, EXITING, DONE };

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();
	
	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	glm::vec2 getPosition() const;
	bool isDoorInteractionStarted() const;
	bool hasDoorTransitionEnded() const;
    bool isTubeEnterStarted() const;
	bool isTubeTraveling() const;
	bool isTubeDone() const;
	void setTubeExitPos(const glm::ivec2 &exitPos);
   void setTubeExitFromTop(bool fromTop);
	void startTubeExit();
	void resetTubeState();
  void startDoorExitAnimation();
	void resetDoorState();
	
private:
	bool bJumping;
	bool facingRight;
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY;
	Texture spritesheet, healthTexture;
	Sprite *sprite, *healthSprite;
	TileMap *map;
	int health;
  DoorState doorState;
	int doorTimer;
	TubeState tubeState;
	int tubeTimer;
	glm::ivec2 tubeExitPos;
  bool tubeExitFromTop;
	bool tubeInputLocked;
 static const int TUBE_ENTER_TIME = 1000;
	static const int TUBE_EXIT_TIME = 400;

};


#endif // _PLAYER_INCLUDE


