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
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT
};

namespace
{
	const int DOOR_ENTER_DURATION_MS = 1000;
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
	//spritesheet.loadFromFile("images/bub.png", TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.loadFromFile("images/bbunny-sprites.png", TEXTURE_PIXEL_FORMAT_RGBA);
	healthTexture.loadFromFile("images/heart.png", TEXTURE_PIXEL_FORMAT_RGBA);
	//sprite = Sprite::createSprite(glm::ivec2(16, 16), glm::vec2(0.25, 0.25), &spritesheet, &shaderProgram);
	sprite = Sprite::createSprite(glm::ivec2(14, 24),
		glm::vec2(14.f / 263.f, 24.f / 174.f),
		&spritesheet, &shaderProgram);
	healthSprite = Sprite::createSprite(glm::ivec2(16, 16), glm::vec2(1.0, 1.0), &healthTexture, &shaderProgram);
	#define OFFSET_X 7.f
	#define UV(col, row) glm::vec2((col * 14.f + OFFSET_X) / 263.f, (row) * 24.f/174.f)

	sprite->setNumberAnimations(4);
	
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
	sprite->addKeyframe(STAND_LEFT, UV(4, 0));

	sprite->setAnimationSpeed(STAND_RIGHT, 8);
	sprite->addKeyframe(STAND_RIGHT, UV(4, 0));

	// Walk: cols 3, 2, 1, 0 en orden
	sprite->setAnimationSpeed(MOVE_LEFT, 8);
	sprite->addKeyframe(MOVE_LEFT, UV(3, 0));
	sprite->addKeyframe(MOVE_LEFT, UV(2, 0));
	sprite->addKeyframe(MOVE_LEFT, UV(1, 0));
	sprite->addKeyframe(MOVE_LEFT, UV(0, 0));

	sprite->setAnimationSpeed(MOVE_RIGHT, 8);
	sprite->addKeyframe(MOVE_RIGHT, UV(3, 0));
	sprite->addKeyframe(MOVE_RIGHT, UV(2, 0));
	sprite->addKeyframe(MOVE_RIGHT, UV(1, 0));
	sprite->addKeyframe(MOVE_RIGHT, UV(0, 0));
		
	sprite->changeAnimation(0);
	tileMapDispl = tileMapPos;
   sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
	
}

void Player::update(int deltaTime)
{
  if(doorState == DoorState::ENTERING)
	{
		doorTimer -= deltaTime;
		if(doorTimer <= 0)
			doorState = DoorState::ENTERED;
       sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
		return;
	}

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
	}
	
	bool isTouchingStair = map->isStairTile(posPlayer);
	if (isTouchingStair)
	{
		if (Game::instance().getKey(GLFW_KEY_SPACE))	cout << "Player is touching a stair tile and space is pressed." << endl;
		if (Game::instance().getKey(GLFW_KEY_UP))	
		{
			posPlayer.y -= 2;
			if (!map->isStairTile(posPlayer))	posPlayer.y += 2;
		}
		if (Game::instance().getKey(GLFW_KEY_DOWN))	
		{
			posPlayer.y += 2;
			map->collisionMoveDown(posPlayer, glm::ivec2(16, 16), &posPlayer.y);
		}
	}
	else 
	{
		posPlayer.y += FALL_STEP;
		map->collisionMoveDown(posPlayer, glm::ivec2(16, 16), &posPlayer.y);
	}

	if(!isTouchingStair && Game::instance().getKey(GLFW_KEY_UP) && map->isDoorTile(posPlayer))
	{
		doorState = DoorState::ENTERING;
		doorTimer = DOOR_ENTER_DURATION_MS;
	}
	
   sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y) + PLAYER_VISUAL_OFFSET_Y_PX));
}

void Player::render()
{
  if(facingRight)
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
	return glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y));
}

bool Player::isDoorInteractionStarted() const
{
	return doorState == DoorState::ENTERING;
}

bool Player::hasDoorTransitionEnded() const
{
	return doorState == DoorState::ENTERED;
}

void Player::resetDoorState()
{
	doorState = DoorState::NONE;
	doorTimer = 0;
}


