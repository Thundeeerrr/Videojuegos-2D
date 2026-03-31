#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FALL_STEP 4


enum PlayerAnims
{
  STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, DOOR_ENTER, DOOR_EXIT, PLANT_CLIMB_UP, PLANT_CLIMB_DOWN, WARP_DISAPPEAR, WARP_APPEAR, JUMP_RIGHT, JUMP_LEFT
};

namespace
{
    const int DOOR_ANIM_DURATION_MS = 750;
    const int DOOR_ANIM_FRAMES = 2;
	const int DOOR_ANIM_FPS = 20;
	const int DOOR_TELEPORT_ANIM_DURATION_MS = (1000 * DOOR_ANIM_FRAMES) / DOOR_ANIM_FPS;
	const int DOOR_ENTER_ROW = 2;
	const int DOOR_ENTER_COL_A = 0;
	const int DOOR_ENTER_COL_B = 1;
   const int DOOR_EXIT_ROW = 2;
	const int DOOR_EXIT_COL_A = 2;
	const int DOOR_EXIT_COL_B = 3;
   const int PLANT_CLIMB_ROW = 7;
   const int PLANT_CLIMB_COL = 0;
   const int PLANT_CLIMB_COL_B = 1;
    const float PLAYER_FRAME_WIDTH_PX = 14.f;
	const float PLAYER_COLLISION_HEIGHT_PX = 16.f;
	const float PLAYER_SPRITE_HEIGHT_PX = 24.f;
	const float PLAYER_VISUAL_OFFSET_Y_PX = PLAYER_COLLISION_HEIGHT_PX - PLAYER_SPRITE_HEIGHT_PX;
  const int DOOR_TELEPORT_EXIT_OFFSET_PX = int(PLAYER_COLLISION_HEIGHT_PX / 2.f);
   const int TUBE_UP_REACH_OFFSET_PX = 16;
    const int WARP_ANIM_FRAMES = 4;
    const int WARP_ANIM_FPS = 6;
	const int WARP_ANIM_DURATION_MS = (1000 * WARP_ANIM_FRAMES) / WARP_ANIM_FPS;
	const int WARP_ANIM_ROW = 3;
}


Player::Player()
{
	sprite = NULL;
	map = NULL;
}

const float Player::JUMP_PLATFORM_VEL = std::sqrt(2.f * Player::JUMP_PLATFORM_GRAVITY * Player::JUMP_PLATFORM_TARGET_HEIGHT_PX);

Player::~Player()
{
	if (sprite != NULL)
		delete sprite;
	if (healthSprite != NULL)
		delete healthSprite;
}

void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	health = 3;
   keyCount = 0;
	bJumping = false;
	facingRight = true;
  doorState = DoorState::NONE;
	doorTimer = 0;
  doorTeleportState = DoorTeleportState::NONE;
	doorTeleportTimer = 0;
 doorTeleportDuration = 0;
	doorTeleportStartPos = glm::ivec2(0);
	doorTeleportTargetPos = glm::ivec2(0);
	doorTeleportExitStartPos = glm::ivec2(0);
 doorTeleportExitFromTop = false;
	doorTeleportDestinationPos = glm::ivec2(0);
    tubeState = TubeState::NONE;
	tubeTimer = 0;
	tubeExitPos = glm::ivec2(0);
    tubeExitFromTop = false;
    tubeInputLocked = false;
    warpState = WarpState::NONE;
	warpTimer = 0;
	warpDestinationPos = glm::ivec2(0);
    jumpPlatformPosY = 0.f;
	//spritesheet.loadFromFile("images/bub.png", TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.loadFromFile("images/BugsBunny-Sprites.png", TEXTURE_PIXEL_FORMAT_RGBA);
	healthTexture.loadFromFile("images/heart.png", TEXTURE_PIXEL_FORMAT_RGBA);
	//sprite = Sprite::createSprite(glm::ivec2(16, 16), glm::vec2(0.25, 0.25), &spritesheet, &shaderProgram);
	sprite = Sprite::createSprite(glm::ivec2(16, 24),
		glm::vec2(1.f / 8.f, 1.f / 12.f),
		&spritesheet, &shaderProgram);
	healthSprite = Sprite::createSprite(glm::ivec2(16, 16), glm::vec2(1.0, 1.0), &healthTexture, &shaderProgram);
	#define UV(col, row) glm::vec2((col) / 8.f, (row) / 12.f)

    sprite->setNumberAnimations(12);
	
		/*sprite->setAnimationSpeed(STAND_LEFT, 8);
		sprite->addKeyframe(STAND_LEFT, glm::vec2(0.f, 0.f));
		
		sprite->setAnimationSpeed(STAND_RIGHT, 8);
		sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.25f, 0.f));
		
		sprite->setAnimationSpeed(MOVE_LEFT, 8);
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.f, 0.f));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.f, 0.25f));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.f, 0.5f));
		
		sprite->setAnimationSpeed(MOVE_RIGHT, 8);
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.25, 0.f));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.25, 0.25f));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.25, 0.5f));*/

	sprite->setAnimationSpeed(STAND_LEFT, 8);
	sprite->addKeyframe(STAND_LEFT, UV(0, 0));

	sprite->setAnimationSpeed(STAND_RIGHT, 8);
	sprite->addKeyframe(STAND_RIGHT, UV(0, 0));

	// Walk: cols 3, 2, 1, 0 en orden
	sprite->setAnimationSpeed(MOVE_LEFT, 8);
	sprite->addKeyframe(MOVE_LEFT, UV(1, 0));
	sprite->addKeyframe(MOVE_LEFT, UV(2, 0));
	sprite->addKeyframe(MOVE_LEFT, UV(3, 0));
	sprite->addKeyframe(MOVE_LEFT, UV(4, 0));

	sprite->setAnimationSpeed(MOVE_RIGHT, 8);
	sprite->addKeyframe(MOVE_RIGHT, UV(1, 0));
	sprite->addKeyframe(MOVE_RIGHT, UV(2, 0));
	sprite->addKeyframe(MOVE_RIGHT, UV(3, 0));
	sprite->addKeyframe(MOVE_RIGHT, UV(4, 0));

	sprite->setAnimationSpeed(DOOR_ENTER, 10);
	sprite->addKeyframe(DOOR_ENTER, UV(DOOR_ENTER_COL_A, DOOR_ENTER_ROW));
	sprite->addKeyframe(DOOR_ENTER, UV(DOOR_ENTER_COL_B, DOOR_ENTER_ROW));

	sprite->setAnimationSpeed(DOOR_EXIT, 10);
	sprite->addKeyframe(DOOR_EXIT, UV(DOOR_EXIT_COL_A, DOOR_EXIT_ROW));
	sprite->addKeyframe(DOOR_EXIT, UV(DOOR_EXIT_COL_B, DOOR_EXIT_ROW));

	sprite->setAnimationSpeed(PLANT_CLIMB_UP, 8);
	sprite->addKeyframe(PLANT_CLIMB_UP, UV(PLANT_CLIMB_COL, PLANT_CLIMB_ROW));
	sprite->addKeyframe(PLANT_CLIMB_UP, UV(PLANT_CLIMB_COL_B, PLANT_CLIMB_ROW));

	sprite->setAnimationSpeed(PLANT_CLIMB_DOWN, 8);
    sprite->addKeyframe(PLANT_CLIMB_DOWN, UV(PLANT_CLIMB_COL_B, PLANT_CLIMB_ROW));
	sprite->addKeyframe(PLANT_CLIMB_DOWN, UV(PLANT_CLIMB_COL, PLANT_CLIMB_ROW));

	sprite->setAnimationSpeed(WARP_DISAPPEAR, WARP_ANIM_FPS);
	sprite->addKeyframe(WARP_DISAPPEAR, UV(0, WARP_ANIM_ROW));
	sprite->addKeyframe(WARP_DISAPPEAR, UV(1, WARP_ANIM_ROW));
	sprite->addKeyframe(WARP_DISAPPEAR, UV(2, WARP_ANIM_ROW));
	sprite->addKeyframe(WARP_DISAPPEAR, UV(3, WARP_ANIM_ROW));

	sprite->setAnimationSpeed(WARP_APPEAR, WARP_ANIM_FPS);
	sprite->addKeyframe(WARP_APPEAR, UV(3, WARP_ANIM_ROW));
	sprite->addKeyframe(WARP_APPEAR, UV(2, WARP_ANIM_ROW));
	sprite->addKeyframe(WARP_APPEAR, UV(1, WARP_ANIM_ROW));
	sprite->addKeyframe(WARP_APPEAR, UV(0, WARP_ANIM_ROW));

	sprite->setAnimationSpeed(JUMP_RIGHT, 8);
	sprite->addKeyframe(JUMP_RIGHT, UV(0, 4));
	sprite->addKeyframe(JUMP_RIGHT, UV(1, 4));

	sprite->setAnimationSpeed(JUMP_LEFT, 8);
	sprite->addKeyframe(JUMP_LEFT, UV(2, 4));
	sprite->addKeyframe(JUMP_LEFT, UV(3, 4));
		
	sprite->changeAnimation(0);
	//tileMapDispl = tileMapPos;
	//sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
	
}

void Player::update(int deltaTime)
{
 if(warpState == WarpState::DISAPPEARING)
	{
		sprite->setAnimationPaused(false);
		if(sprite->animation() != WARP_DISAPPEAR)
			sprite->changeAnimation(WARP_DISAPPEAR);
		sprite->update(deltaTime);
		warpTimer -= deltaTime;
		if(warpTimer <= 0)
		{
			posPlayer = warpDestinationPos;
			warpState = WarpState::APPEARING;
			warpTimer = WARP_ANIM_DURATION_MS;
		}
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
		return;
	}
	if(warpState == WarpState::APPEARING)
	{
		sprite->setAnimationPaused(false);
		if(sprite->animation() != WARP_APPEAR)
			sprite->changeAnimation(WARP_APPEAR);
		sprite->update(deltaTime);
		warpTimer -= deltaTime;
		if(warpTimer <= 0)
        {
			warpState = WarpState::NONE;
			sprite->changeAnimation(facingRight ? STAND_RIGHT : STAND_LEFT);
		}
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
		return;
	}
   auto lerpPos = [](const glm::ivec2 &from, const glm::ivec2 &to, float t) -> glm::ivec2
	{
		float fx = float(from.x) + (float(to.x - from.x) * t);
		float fy = float(from.y) + (float(to.y - from.y) * t);
		return glm::ivec2(int(std::round(fx)), int(std::round(fy)));
	};
 if(doorTeleportState == DoorTeleportState::ENTERING)
	{
		sprite->setAnimationPaused(false);
		if(sprite->animation() != DOOR_ENTER)
			sprite->changeAnimation(DOOR_ENTER);
		sprite->update(deltaTime);
		doorTeleportTimer -= deltaTime;
      float t = 1.f - (float(doorTeleportTimer) / float(doorTeleportDuration));
		if(t < 0.f) t = 0.f;
		if(t > 1.f) t = 1.f;
		posPlayer = lerpPos(doorTeleportStartPos, doorTeleportTargetPos, t);
		if(doorTeleportTimer <= 0)
		{
         posPlayer = doorTeleportExitStartPos;
			doorTeleportState = DoorTeleportState::EXITING;
           doorTeleportTimer = DOOR_TELEPORT_ANIM_DURATION_MS;
			doorTeleportDuration = DOOR_TELEPORT_ANIM_DURATION_MS;
			doorTeleportStartPos = doorTeleportExitStartPos;
			doorTeleportTargetPos = doorTeleportDestinationPos;
			sprite->changeAnimation(DOOR_EXIT);
		}
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
		return;
	}
	if(doorTeleportState == DoorTeleportState::EXITING)
	{
		sprite->setAnimationPaused(false);
		if(sprite->animation() != DOOR_EXIT)
			sprite->changeAnimation(DOOR_EXIT);
		sprite->update(deltaTime);
		doorTeleportTimer -= deltaTime;
      float t = 1.f - (float(doorTeleportTimer) / float(doorTeleportDuration));
		if(t < 0.f) t = 0.f;
		if(t > 1.f) t = 1.f;
		posPlayer = lerpPos(doorTeleportStartPos, doorTeleportTargetPos, t);
		if(doorTeleportTimer <= 0)
		{
			doorTeleportState = DoorTeleportState::NONE;
            posPlayer = doorTeleportDestinationPos;
			sprite->changeAnimation(facingRight ? STAND_RIGHT : STAND_LEFT);
		}
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
		return;
	}
    if(bJumpPlatformActive)
	{
		sprite->setAnimationPaused(false);
		int jumpAnim = facingRight ? JUMP_RIGHT : JUMP_LEFT;
		if(sprite->animation() != jumpAnim)
			sprite->changeAnimation(jumpAnim);
	}
	if(tubeState == TubeState::ENTERING)
	{
		if(sprite->animation() != (facingRight ? STAND_RIGHT : STAND_LEFT))
			sprite->changeAnimation(facingRight ? STAND_RIGHT : STAND_LEFT);
		sprite->setAnimationPaused(false);
		sprite->update(deltaTime);
		tubeTimer -= deltaTime;
		if(tubeTimer <= 0)
			tubeState = TubeState::TRAVELING;
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
		return;
	}
	if(tubeState == TubeState::TRAVELING)
	{
		if(sprite->animation() != (facingRight ? STAND_RIGHT : STAND_LEFT))
			sprite->changeAnimation(facingRight ? STAND_RIGHT : STAND_LEFT);
		sprite->setAnimationPaused(false);
		sprite->update(deltaTime);
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
		return;
	}
	if(tubeState == TubeState::EXITING)
	{
		if(sprite->animation() != (facingRight ? STAND_RIGHT : STAND_LEFT))
			sprite->changeAnimation(facingRight ? STAND_RIGHT : STAND_LEFT);
		sprite->setAnimationPaused(false);
		sprite->update(deltaTime);

		float t = 1.f - (float(tubeTimer) / float(TUBE_EXIT_TIME));
		if(t < 0.f) t = 0.f;
		if(t > 1.f) t = 1.f;
       float startY = float(tubeExitPos.y + (tubeExitFromTop ? -16 : 16));
		float endY = float(tubeExitPos.y);
		posPlayer.y = int(startY + (endY - startY) * t);

		tubeTimer -= deltaTime;
		if(tubeTimer <= 0)
		{
			posPlayer.y = tubeExitPos.y;
			tubeState = TubeState::DONE;
		}
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
		return;
	}
	if(tubeState == TubeState::DONE)
	{
		if(sprite->animation() != (facingRight ? STAND_RIGHT : STAND_LEFT))
			sprite->changeAnimation(facingRight ? STAND_RIGHT : STAND_LEFT);
		sprite->setAnimationPaused(false);
		sprite->update(deltaTime);
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
		return;
	}

  if(doorState == DoorState::ENTERING || doorState == DoorState::LEAVING)
	{
     sprite->setAnimationPaused(false);
     sprite->update(deltaTime);
		int doorAnim = (doorState == DoorState::ENTERING) ? DOOR_ENTER : DOOR_EXIT;
		if(sprite->animation() != doorAnim)
			sprite->changeAnimation(doorAnim);

		doorTimer -= deltaTime;
		if(doorTimer <= 0)
     {
			if(doorState == DoorState::ENTERING)
				doorState = DoorState::ENTERED;
			else
			{
				doorState = DoorState::NONE;
				facingRight = true;
				sprite->changeAnimation(STAND_RIGHT);
			}
		}
       sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
		return;
	}

    bool upPressed = Game::instance().getKey(GLFW_KEY_UP);
	bool downPressed = Game::instance().getKey(GLFW_KEY_DOWN);
 if(tubeState == TubeState::NONE && !upPressed && !downPressed)
		tubeInputLocked = false;
	bool isTouchingStair = map->isStairTile(posPlayer);
	bool isClimbAnim = (sprite->animation() == PLANT_CLIMB_UP || sprite->animation() == PLANT_CLIMB_DOWN);
	sprite->setAnimationPaused(isTouchingStair && !upPressed && !downPressed && isClimbAnim);

	sprite->update(deltaTime);
  bool leftPressed = Game::instance().getKey(GLFW_KEY_LEFT);
	bool rightPressed = Game::instance().getKey(GLFW_KEY_RIGHT);
 int moveStep = bJumpPlatformActive ? 1 : 2;
 if(leftPressed)
	{
        facingRight = false;
        if(!bJumpPlatformActive && sprite->animation() != MOVE_LEFT)
			sprite->changeAnimation(MOVE_LEFT);
       posPlayer.x -= moveStep;
		if(map->collisionMoveLeft(posPlayer, glm::ivec2(16, 16)))
		{
           posPlayer.x += moveStep;
            if(!bJumpPlatformActive)
				sprite->changeAnimation(STAND_LEFT);
		}
	}
    else if(rightPressed)
	{
       facingRight = true;
       if(!bJumpPlatformActive && sprite->animation() != MOVE_RIGHT)
			sprite->changeAnimation(MOVE_RIGHT);
       posPlayer.x += moveStep;
		if(map->collisionMoveRight(posPlayer, glm::ivec2(16, 16)))
		{
           posPlayer.x -= moveStep;
           if(!bJumpPlatformActive)
				sprite->changeAnimation(STAND_RIGHT);
		}
	}
	else
	{
        if(!bJumpPlatformActive && sprite->animation() == MOVE_LEFT)
			sprite->changeAnimation(STAND_LEFT);
      else if(!bJumpPlatformActive && sprite->animation() == MOVE_RIGHT)
			sprite->changeAnimation(STAND_RIGHT);
       else if(!bJumpPlatformActive && (sprite->animation() == DOOR_ENTER || sprite->animation() == DOOR_EXIT))
			sprite->changeAnimation(STAND_RIGHT);
	}
	
    bool onGround = false;
    bool horizontalPressed = leftPressed || rightPressed;
	if (bJumpPlatformActive)
	{
       jumpPlatformPosY += jumpVelocity;
		posPlayer.y = int(std::round(jumpPlatformPosY));
		jumpVelocity += JUMP_PLATFORM_GRAVITY;
       if(horizontalPressed)
		{
			if(jumpPlatformInputReleased)
				jumpVelocity += JUMP_PLATFORM_DAMP;
		}
		else
			jumpPlatformInputReleased = true;

		int adjustedY = posPlayer.y;
		if(map->collisionMoveUp(posPlayer, glm::ivec2(16, 16), &adjustedY))
		{
			posPlayer.y = adjustedY;
         jumpPlatformPosY = float(posPlayer.y);
			jumpVelocity = 0.f;
			bJumpPlatformActive = false;
		}
     if(jumpVelocity >= 0.f)
		{
			jumpPlatformPosY = float(posPlayer.y);
			bJumpPlatformActive = false;
       }
	}
	else if (isTouchingStair)
	{
		if (Game::instance().getKey(GLFW_KEY_SPACE))	cout << "Player is touching a stair tile and space is pressed." << endl;
        if (upPressed)	
		{
           if(sprite->animation() != PLANT_CLIMB_UP)
				sprite->changeAnimation(PLANT_CLIMB_UP);
			posPlayer.y -= 2;
			if (!map->isStairTile(posPlayer))	posPlayer.y += 2;
		}
        if (downPressed)	
		{
           if(sprite->animation() != PLANT_CLIMB_DOWN)
				sprite->changeAnimation(PLANT_CLIMB_DOWN);
			posPlayer.y += 2;
			map->collisionMoveDown(posPlayer, glm::ivec2(16, 16), &posPlayer.y);
		}
       onGround = true;
	}
	else 
	{
		posPlayer.y += FALL_STEP;
        onGround = map->collisionMoveDown(posPlayer, glm::ivec2(16, 16), &posPlayer.y);
	}
	if(!bJumpPlatformActive)
		jumpPlatformPosY = float(posPlayer.y);

	const int playerW = 16;
	const int playerH = 16;
	const int tileSize = map->getTileSize();
	auto getFeetTile = [&](const glm::ivec2 &playerPos) -> glm::ivec2
	{
		int probeX = playerPos.x + playerW / 2;
		int probeY = playerPos.y + playerH;
		return glm::ivec2(probeX / tileSize, probeY / tileSize);
	};
  glm::ivec2 playerTile = glm::ivec2(
		int((posPlayer.x + playerW / 2.f) / tileSize),
		int((posPlayer.y + playerH - 1.f) / tileSize));
	auto isDoorObjectTile = [&](const glm::ivec2 &tilePos) -> bool
	{
		const std::vector<glm::ivec2> &doorTiles = map->getDoorPositions();
		for(int i = 0; i < int(doorTiles.size()); ++i)
		{
			if(doorTiles[i] == tilePos)
				return true;
		}
		return false;
	};

	if(onGround && !bJumpPlatformActive)
	{
		glm::ivec2 feetTile = getFeetTile(posPlayer);
     if(map->getTile(feetTile.x, feetTile.y) == TileMap::JUMP_PLATFORM_TILE)
		{
          jumpPlatformPosY = float(posPlayer.y);
			jumpVelocity = -JUMP_PLATFORM_VEL;
			bJumpPlatformActive = true;
           jumpPlatformInputReleased = !horizontalPressed;
		}
	}

 glm::ivec2 warpTile = getFeetTile(posPlayer);
	int warpTileId = map->getTile(warpTile.x, warpTile.y);
	bool isWarpTile = warpTileId == TileMap::WARP_TILE_FLOOR || warpTileId == TileMap::WARP_TILE_NO_FLOOR;
 if(downPressed && !bWarpUsed && isWarpTile && doorState == DoorState::NONE && tubeState == TubeState::NONE)
	{
		auto warpPairs = map->getWarpPlatformPairs();
		glm::ivec2 destinationTile = warpTile;
		bool foundDestination = false;
		for(const auto &pair : warpPairs)
		{
			if(pair.first == warpTile)
			{
				destinationTile = pair.second;
				foundDestination = true;
				break;
			}
			if(pair.second == warpTile)
			{
				destinationTile = pair.first;
				foundDestination = true;
				break;
			}
		}

        if(foundDestination)
		{
          warpDestinationPos.x = destinationTile.x * tileSize + (tileSize - playerW) / 2;
			warpDestinationPos.y = destinationTile.y * tileSize - playerH;
			warpState = WarpState::DISAPPEARING;
			warpTimer = WARP_ANIM_DURATION_MS;
			bWarpUsed = true;
		}
	}

	if(map->isDoorTile(playerTile) && upPressed && !bWarpUsed)
	{
		glm::ivec2 destinationTile = map->getDoorDestination(playerTile);
		if(destinationTile.x != -1)
		{
         doorTeleportDestinationPos = glm::ivec2(
				destinationTile.x * tileSize,
				destinationTile.y * tileSize - playerH + tileSize);
            doorTeleportStartPos = posPlayer;
			doorTeleportTargetPos = glm::ivec2(
				playerTile.x * tileSize,
				playerTile.y * tileSize);
          int destinationTileId = map->getTile(destinationTile.x, destinationTile.y);
			doorTeleportExitFromTop = destinationTileId == TileMap::DOOR_TILE_STAIRS;
			if(doorTeleportExitFromTop)
			{
				doorTeleportExitStartPos = glm::ivec2(
					destinationTile.x * tileSize,
					destinationTile.y * tileSize - playerH + tileSize - DOOR_TELEPORT_EXIT_OFFSET_PX);
			}
			else
			{
				doorTeleportExitStartPos = glm::ivec2(
					destinationTile.x * tileSize,
					destinationTile.y * tileSize - playerH + tileSize + DOOR_TELEPORT_EXIT_OFFSET_PX);
			}
			doorTeleportState = DoorTeleportState::ENTERING;
           doorTeleportTimer = DOOR_TELEPORT_ANIM_DURATION_MS;
			doorTeleportDuration = DOOR_TELEPORT_ANIM_DURATION_MS;
			sprite->changeAnimation(DOOR_ENTER);
			bWarpUsed = true;
			sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
			return;
		}
	}

  glm::ivec2 warpTileAfter = getFeetTile(posPlayer);
	int warpTileAfterId = map->getTile(warpTileAfter.x, warpTileAfter.y);
	bool onWarpTile = warpTileAfterId == TileMap::WARP_TILE_FLOOR || warpTileAfterId == TileMap::WARP_TILE_NO_FLOOR;
 bool onDoorTile = map->isDoorTile(playerTile);
	if((!downPressed || !onWarpTile) && (!upPressed || !onDoorTile))
		bWarpUsed = false;

	bool onTubeTop = map->isTubeTile(posPlayer, true);
	bool onTubeBottom = map->isTubeTile(posPlayer, false);
    bool onTubeBottomAbove = map->isTubeTile(glm::ivec2(posPlayer.x, posPlayer.y - TUBE_UP_REACH_OFFSET_PX), false);
    if(upPressed || downPressed)
        cout << "[TubeDebug] posPlayer=(" << posPlayer.x << "," << posPlayer.y << ") onTop=" << onTubeTop << " onBottom=" << onTubeBottom << " onBottomAbove=" << onTubeBottomAbove << " stair=" << isTouchingStair << " doorState=" << int(doorState) << " tubeLock=" << tubeInputLocked << endl;
	bool tubeActivationPressed = (onTubeTop && downPressed) || ((onTubeBottom || onTubeBottomAbove) && upPressed);
  if(!tubeInputLocked && !isTouchingStair && doorState == DoorState::NONE && tubeActivationPressed)
	{
       cout << "[TubeDebug] Activation accepted. Entering travel from pos=(" << posPlayer.x << "," << posPlayer.y << ")" << endl;
       tubeState = TubeState::ENTERING;
		tubeTimer = TUBE_ENTER_TIME;
     tubeInputLocked = true;
		if(sprite->animation() != (facingRight ? STAND_RIGHT : STAND_LEFT))
			sprite->changeAnimation(facingRight ? STAND_RIGHT : STAND_LEFT);
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
		return;
	}

   if(!isTouchingStair && upPressed && isDoorObjectTile(playerTile))
	{
		doorState = DoorState::ENTERING;
     doorTimer = DOOR_ANIM_DURATION_MS;
       sprite->changeAnimation(DOOR_ENTER);
	}
	
   sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
}

void Player::render()
{
  if(tubeState == TubeState::ENTERING || tubeState == TubeState::TRAVELING)
	{
		glm::vec2 healthPos = glm::vec2(0.0f, 0.0f);
		for (int i = 0; i < health; i++)
		{
			healthSprite->setPosition(healthPos);
			healthSprite->render();
			healthPos.x += 15.f;
		}
		return;
	}

  int currentAnim = sprite->animation();
	bool usesExplicitFacing = currentAnim == JUMP_LEFT || currentAnim == JUMP_RIGHT;
	if(!facingRight && !usesExplicitFacing)
	{
		glm::mat4 local = glm::translate(glm::mat4(1.0f), glm::vec3(PLAYER_FRAME_WIDTH_PX, 0.f, 0.f));
		local = glm::scale(local, glm::vec3(-1.f, 1.f, 1.f));
		sprite->setLocalTransform(local);
	}
	else
		sprite->setLocalTransform(glm::mat4(1.0f));

	sprite->render();
	glm::vec2 healthPos = glm::vec2(0.0f, 0.0f);
	for (int i = 0; i < health; i++)
	{
		healthSprite->setPosition(healthPos);
		healthSprite->render();
		healthPos.x += 15.f;
	}
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	posPlayer = pos;
    jumpPlatformPosY = float(posPlayer.y);
   sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
}

glm::vec2 Player::getPosition() const
{
 return glm::vec2(float(posPlayer.x), float(posPlayer.y));
}

glm::ivec2 Player::getSize() const
{
	return glm::ivec2(16, 16);
}

void Player::addKey()
{
	++keyCount;
}

int Player::getKeyCount() const
{
	return keyCount;
}

bool Player::isDoorInteractionStarted() const
{
	return doorState == DoorState::ENTERING;
}

bool Player::hasDoorTransitionEnded() const
{
	return doorState == DoorState::ENTERED;
}

bool Player::isTubeEnterStarted() const
{
	return tubeState == TubeState::ENTERING;
}

bool Player::isTubeTraveling() const
{
	return tubeState == TubeState::TRAVELING;
}

bool Player::isTubeDone() const
{
	return tubeState == TubeState::DONE;
}

void Player::setTubeExitPos(const glm::ivec2 &exitPos)
{
	tubeExitPos = exitPos;
}

void Player::setTubeExitFromTop(bool fromTop)
{
	tubeExitFromTop = fromTop;
}

void Player::startTubeExit()
{
	tubeState = TubeState::EXITING;
	tubeTimer = TUBE_EXIT_TIME;
}

void Player::resetTubeState()
{
	tubeState = TubeState::NONE;
	tubeTimer = 0;
   tubeInputLocked = true;
}

void Player::resetDoorState()
{
	doorState = DoorState::NONE;
	doorTimer = 0;
}

void Player::startDoorExitAnimation()
{
	doorState = DoorState::LEAVING;
 doorTimer = DOOR_ANIM_DURATION_MS;
	sprite->changeAnimation(DOOR_EXIT);
}


