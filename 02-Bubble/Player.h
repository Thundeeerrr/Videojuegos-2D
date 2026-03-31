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
  enum class DoorTeleportState { NONE, ENTERING, EXITING };
	enum class TubeState { NONE, ENTERING, TRAVELING, EXITING, DONE };
	enum class WarpState { NONE, DISAPPEARING, APPEARING };

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();
	
	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	glm::vec2 getPosition() const;
    glm::ivec2 getSize() const;
  void addKey();
	int getKeyCount() const;
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
	void startDeathAnimation();
	bool isDeathAnimationFinished() const;
	void setLives(int lives);
	int getLives() const;
	void activateShield();
	void consumeShield();
	bool hasShield() const;
	void setGodModeShieldVisual(bool enabled);
	
private:
	bool bJumping;
	bool facingRight;
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY;
 Texture spritesheet, healthTexture, shieldTexture;
	Sprite *sprite, *healthSprite, *shieldSprite;
	TileMap *map;
	int health;
  int keyCount;
  DoorState doorState;
	int doorTimer;
    DoorTeleportState doorTeleportState;
	int doorTeleportTimer;
  int doorTeleportDuration;
	glm::ivec2 doorTeleportStartPos;
	glm::ivec2 doorTeleportTargetPos;
	glm::ivec2 doorTeleportExitStartPos;
  bool doorTeleportExitFromTop;
	glm::ivec2 doorTeleportDestinationPos;
	TubeState tubeState;
	int tubeTimer;
	glm::ivec2 tubeExitPos;
  bool tubeExitFromTop;
	bool tubeInputLocked;
  bool bJumpPlatformActive = false;
	float jumpVelocity = 0.f;
  float jumpPlatformPosY = 0.f;
  bool jumpPlatformInputReleased = true;
  WarpState warpState;
	int warpTimer;
	glm::ivec2 warpDestinationPos;
   bool bWarpUsed = false;
   bool deathActive;
	bool deathFinished;
	int deathTimerMs;
   bool shieldActive;
   bool godModeShieldVisual;
 static const int TUBE_ENTER_TIME = 1000;
	static const int TUBE_EXIT_TIME = 400;
    static constexpr float JUMP_PLATFORM_GRAVITY = 0.1f;
	static constexpr float JUMP_PLATFORM_TARGET_HEIGHT_PX = 112.f;
	static const float JUMP_PLATFORM_VEL;
	static constexpr float JUMP_PLATFORM_DAMP = 0.4f;

};


#endif // _PLAYER_INCLUDE


