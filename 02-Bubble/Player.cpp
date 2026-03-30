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
  STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, DOOR_ENTER, DOOR_EXIT, PLANT_CLIMB_UP, PLANT_CLIMB_DOWN
};

namespace
{
    const int DOOR_ANIM_DURATION_MS = 750;
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
}


Player::Player()
{
	sprite = NULL;
	map = NULL;
}

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
	bJumping = false;
	facingRight = true;
  doorState = DoorState::NONE;
	doorTimer = 0;
    tubeState = TubeState::NONE;
	tubeTimer = 0;
	tubeExitPos = glm::ivec2(0);
    tubeExitFromTop = false;
    tubeInputLocked = false;
	//spritesheet.loadFromFile("images/bub.png", TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.loadFromFile("images/BugsBunny-Sprites.png", TEXTURE_PIXEL_FORMAT_RGBA);
	healthTexture.loadFromFile("images/heart.png", TEXTURE_PIXEL_FORMAT_RGBA);
	//sprite = Sprite::createSprite(glm::ivec2(16, 16), glm::vec2(0.25, 0.25), &spritesheet, &shaderProgram);
	sprite = Sprite::createSprite(glm::ivec2(16, 24),
		glm::vec2(1.f / 8.f, 1.f / 12.f),
		&spritesheet, &shaderProgram);
	healthSprite = Sprite::createSprite(glm::ivec2(16, 16), glm::vec2(1.0, 1.0), &healthTexture, &shaderProgram);
	#define UV(col, row) glm::vec2((col) / 8.f, (row) / 12.f)

 sprite->setNumberAnimations(8);
	
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
		
	sprite->changeAnimation(0);
	tileMapDispl = tileMapPos;
   sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
	
}

void Player::update(int deltaTime)
{
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
	if(Game::instance().getKey(GLFW_KEY_LEFT))
	{
        facingRight = false;
		if(sprite->animation() != MOVE_LEFT)
			sprite->changeAnimation(MOVE_LEFT);
		posPlayer.x -= 2;
		if(map->collisionMoveLeft(posPlayer, glm::ivec2(16, 16)))
		{
			posPlayer.x += 2;
			sprite->changeAnimation(STAND_LEFT);
		}
	}
	else if(Game::instance().getKey(GLFW_KEY_RIGHT))
	{
       facingRight = true;
		if(sprite->animation() != MOVE_RIGHT)
			sprite->changeAnimation(MOVE_RIGHT);
		posPlayer.x += 2;
		if(map->collisionMoveRight(posPlayer, glm::ivec2(16, 16)))
		{
			posPlayer.x -= 2;
			sprite->changeAnimation(STAND_RIGHT);
		}
	}
	else
	{
		if(sprite->animation() == MOVE_LEFT)
			sprite->changeAnimation(STAND_LEFT);
		else if(sprite->animation() == MOVE_RIGHT)
			sprite->changeAnimation(STAND_RIGHT);
       else if(sprite->animation() == DOOR_ENTER || sprite->animation() == DOOR_EXIT)
			sprite->changeAnimation(STAND_RIGHT);
	}
	
	if (isTouchingStair)
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
	}
	else 
	{
		posPlayer.y += FALL_STEP;
		map->collisionMoveDown(posPlayer, glm::ivec2(16, 16), &posPlayer.y);
	}

	bool onTubeTop = map->isTubeTile(posPlayer, true);
	bool onTubeBottom = map->isTubeTile(posPlayer, false);
    if(upPressed || downPressed)
		cout << "[TubeDebug] posPlayer=(" << posPlayer.x << "," << posPlayer.y << ") onTop=" << onTubeTop << " onBottom=" << onTubeBottom << " stair=" << isTouchingStair << " doorState=" << int(doorState) << " tubeLock=" << tubeInputLocked << endl;
    bool tubeArrowPressed = upPressed || downPressed;
  if(!tubeInputLocked && !isTouchingStair && doorState == DoorState::NONE &&
		(onTubeTop || onTubeBottom) && tubeArrowPressed)
	{
       cout << "[TubeDebug] Activation accepted. Entering travel from pos=(" << posPlayer.x << "," << posPlayer.y << ")" << endl;
        tubeState = TubeState::TRAVELING;
		tubeTimer = 0;
     tubeInputLocked = true;
		if(sprite->animation() != (facingRight ? STAND_RIGHT : STAND_LEFT))
			sprite->changeAnimation(facingRight ? STAND_RIGHT : STAND_LEFT);
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
		return;
	}

  if(!isTouchingStair && upPressed && map->isDoorTile(posPlayer))
	{
		doorState = DoorState::ENTERING;
     doorTimer = DOOR_ANIM_DURATION_MS;
       sprite->changeAnimation(DOOR_ENTER);
	}
	
   sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
}

void Player::render()
{
  if(!facingRight)
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
   sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
}

glm::vec2 Player::getPosition() const
{
 return glm::vec2(float(posPlayer.x), float(posPlayer.y));
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


