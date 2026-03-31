#include <iostream>
#include <cmath>
#include <sstream>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"
#include "Utils.h"
#include "DonaldEnemy.h"
#include "PiolinEnemy.h"
#include "FrancoEnemy.h"

#define SCREEN_X 0
#define SCREEN_Y 0

#define INIT_PLAYER_X_TILES 9
#define INIT_PLAYER_Y_TILES 4

namespace
{
 const int PLAYER_CENTER_OFFSET_X_PX = 8;
	const int PLAYER_COLLISION_HEIGHT_PX = 16;
	const int PLAYER_FOOT_OFFSET_Y_PX = PLAYER_COLLISION_HEIGHT_PX;
	const int PLAYER_FOOT_OFFSET_Y_MINUS_ONE_PX = PLAYER_COLLISION_HEIGHT_PX - 1;
  const int PLAYER_HEAD_OFFSET_Y_PX = 0;
	const int PLAYER_HEAD_OFFSET_Y_MINUS_ONE_PX = -1;
	const int EXPLOSION_TILE_SIZE_PX = 16;
	const int EXPLOSION_FRAME_COUNT = 5;
	const int EXPLOSION_DURATION_MS = 400;
  const int GAME_OVER_DURATION_MS = 5000;
	const int GAME_OVER_APPEAR_TIME_MS = 700;
	const float GAME_OVER_DARKEN_FACTOR = 0.45f;
   const float PAUSE_MENU_DARKEN_FACTOR = 0.45f;
	const int HUD_BOMB_ICON_SIZE_PX = 16;
	const int HUD_ICON_SPACING_PX = 15;
    const int HUD_BOMB_ROW_Y_PX = 16;

	bool containsDoorTile(const std::vector<glm::ivec2> &tiles, const glm::ivec2 &tile)
	{
		for(int i=0; i<int(tiles.size()); ++i)
		{
			if(tiles[i] == tile)
				return true;
		}

		return false;
	}
}


Scene::Scene()
{
	map = NULL;
	player = NULL;
   dWasPressed = false;
	hasDoorTarget = false;
	hasSpawnOverride = false;
  spawnAtDoorInLoadedLevel = false;
	doorTargetTilePos = glm::ivec2(0);
	spawnTileOverride = glm::ivec2(0);
    currentLevelNum = 1;
	hasReturnPoint = false;
	returnLevelNum = 1;
  returnTilePos = glm::ivec2(0);
	godMode = false;
 weightPushLatch.clear();
   explosionVao = 0;
	explosionVbo = 0;
	explosionPosLocation = -1;
	explosionTexCoordLocation = -1;
  bombHudVao = 0;
	bombHudVbo = 0;
	bombHudPosLocation = -1;
	bombHudTexCoordLocation = -1;
	playerDeathActive = false;
 gameOverActive = false;
	gameOverTimerMs = 0;
	pauseActive = false;
	pWasPressed = false;
	hWasPressed = false;
   enemyExplosions.clear();
   remainingLives = MAX_LIVES;
   spaceWasPressed = false;
	hasSuspendedLevel = false;
	suspendedLevelNum = -1;
	suspendedMap = NULL;
	suspendedPlayer = NULL;
}

Scene::~Scene()
{
	texProgram.free();
	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;
   freeDoors();
   freeKeys();
   freeEnemies();
    freeBombs();
   if(explosionVao != 0)
		glDeleteVertexArrays(1, &explosionVao);
	if(explosionVbo != 0)
		glDeleteBuffers(1, &explosionVbo);
   if(gameOverVao != 0)
		glDeleteVertexArrays(1, &gameOverVao);
	if(gameOverVbo != 0)
		glDeleteBuffers(1, &gameOverVbo);
  if(bombHudVao != 0)
		glDeleteVertexArrays(1, &bombHudVao);
	if(bombHudVbo != 0)
		glDeleteBuffers(1, &bombHudVbo);
   if(bombHudVao != 0)
		glDeleteVertexArrays(1, &bombHudVao);
	if(bombHudVbo != 0)
		glDeleteBuffers(1, &bombHudVbo);
}


void Scene::init(const std::string &sceneName)
{
	initShaders();
	freeDoors();
	freeKeys();
	freeEnemies();
 freeBombs();
	map = TileMap::createTileMap(sceneName, glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
 map->clearDoorLinks();
   if(currentLevelNum == 2)
	{
		map->addDoorLink(glm::ivec2(12, 1), glm::ivec2(13, 3));
		map->addDoorLink(glm::ivec2(6, 3), glm::ivec2(5, 5));
		map->addDoorLink(glm::ivec2(12, 5), glm::ivec2(13, 7));
	}
	else if(currentLevelNum == 4)
	{
		map->addDoorLink(glm::ivec2(12, 1), glm::ivec2(13, 3));
		map->addDoorLink(glm::ivec2(9, 5), glm::ivec2(10, 7));
	}
	else if(currentLevelNum == 5)
	{
		map->addDoorLink(glm::ivec2(3, 6), glm::ivec2(4, 8));
		map->addDoorLink(glm::ivec2(8, 8), glm::ivec2(7, 10));
	}
	const std::vector<glm::ivec2> &doorPositions = map->getDoorPositions();
	const std::unordered_set<glm::ivec2, TileMap::IVec2Hash> &keyPositions = map->getKeyPositions();
	for(int i=0; i<int(doorPositions.size()); ++i)
	{
		Door *door = new Door();
		door->init(doorPositions[i], texProgram);
		doors.push_back(door);
	}
	for(auto it=keyPositions.begin(); it != keyPositions.end(); ++it)
	{
		Key *key = new Key();
		key->init(*it, texProgram);
		keys.push_back(key);
	}
  const std::vector<glm::ivec2> &weightTiles = map->getWeightPositions();
	for(int i=0; i<int(weightTiles.size()); ++i)
	{
		glm::ivec2 weightWorld(weightTiles[i].x * map->getTileSize(), weightTiles[i].y * map->getTileSize());
		Pushable *weight = new Pushable();
		weight->init(weightWorld, texProgram, map);
		weights.push_back(weight);
	}
    const std::vector<glm::ivec2> &bombTiles = map->getBombPositions();
	for(int i=0; i<int(bombTiles.size()); ++i)
	{
		Bomb *bomb = new Bomb();
		bomb->init(bombTiles[i], texProgram);
		bombs.push_back(bomb);
	}
  weightPushLatch.assign(weights.size(), false);
  playerDeathActive = false;
    gameOverActive = false;
	gameOverTimerMs = 0;
	enemyExplosions.clear();
	spaceWasPressed = false;

   explosionTexture.loadFromFile("images/Explosions.png", TEXTURE_PIXEL_FORMAT_RGBA);
	explosionTexture.setWrapS(GL_CLAMP_TO_EDGE);
	explosionTexture.setWrapT(GL_CLAMP_TO_EDGE);
	explosionTexture.setMinFilter(GL_NEAREST);
	explosionTexture.setMagFilter(GL_NEAREST);

	if(explosionVao != 0)
		glDeleteVertexArrays(1, &explosionVao);
	if(explosionVbo != 0)
		glDeleteBuffers(1, &explosionVbo);

	const float frameWidthUv = 1.f / float(EXPLOSION_FRAME_COUNT);
	float vertices[16] = {
		0.f, 0.f, 0.f, 0.f,
		float(EXPLOSION_TILE_SIZE_PX), 0.f, frameWidthUv, 0.f,
		float(EXPLOSION_TILE_SIZE_PX), float(EXPLOSION_TILE_SIZE_PX), frameWidthUv, 1.f,
		0.f, float(EXPLOSION_TILE_SIZE_PX), 0.f, 1.f
	};

	glGenVertexArrays(1, &explosionVao);
	glBindVertexArray(explosionVao);
	glGenBuffers(1, &explosionVbo);
	glBindBuffer(GL_ARRAY_BUFFER, explosionVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	explosionPosLocation = texProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	explosionTexCoordLocation = texProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));

  bombHudTexture.loadFromFile("images/bomba.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bombHudTexture.setWrapS(GL_CLAMP_TO_EDGE);
	bombHudTexture.setWrapT(GL_CLAMP_TO_EDGE);
	bombHudTexture.setMinFilter(GL_NEAREST);
	bombHudTexture.setMagFilter(GL_NEAREST);

	float bombHudVertices[16] = {
		0.f, 0.f, 0.f, 0.f,
		float(HUD_BOMB_ICON_SIZE_PX), 0.f, 1.f, 0.f,
		float(HUD_BOMB_ICON_SIZE_PX), float(HUD_BOMB_ICON_SIZE_PX), 1.f, 1.f,
		0.f, float(HUD_BOMB_ICON_SIZE_PX), 0.f, 1.f
	};

	glGenVertexArrays(1, &bombHudVao);
	glBindVertexArray(bombHudVao);
	glGenBuffers(1, &bombHudVbo);
	glBindBuffer(GL_ARRAY_BUFFER, bombHudVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bombHudVertices), bombHudVertices, GL_STATIC_DRAW);
	bombHudPosLocation = texProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	bombHudTexCoordLocation = texProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));

	gameOverTexture.loadFromFile("images/Game over.png", TEXTURE_PIXEL_FORMAT_RGBA);
	gameOverTexture.setWrapS(GL_CLAMP_TO_EDGE);
	gameOverTexture.setWrapT(GL_CLAMP_TO_EDGE);
	gameOverTexture.setMinFilter(GL_NEAREST);
	gameOverTexture.setMagFilter(GL_NEAREST);

	if(gameOverVao != 0)
		glDeleteVertexArrays(1, &gameOverVao);
	if(gameOverVbo != 0)
		glDeleteBuffers(1, &gameOverVbo);

  const float mapPixelW = map->getMapSize().x * map->getTileSize();
	const float mapPixelH = map->getMapSize().y * map->getTileSize();
	float gameOverVertices[16] = {
		0.f, 0.f, 0.f, 0.f,
      mapPixelW, 0.f, 1.f, 0.f,
		mapPixelW, mapPixelH, 1.f, 1.f,
		0.f, mapPixelH, 0.f, 1.f
	};

	glGenVertexArrays(1, &gameOverVao);
	glBindVertexArray(gameOverVao);
	glGenBuffers(1, &gameOverVbo);
	glBindBuffer(GL_ARRAY_BUFFER, gameOverVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gameOverVertices), gameOverVertices, GL_STATIC_DRAW);
	gameOverPosLocation = texProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	gameOverTexCoordLocation = texProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));

	pauseMenuTexture.loadFromFile("images/Pause-Menu.png", TEXTURE_PIXEL_FORMAT_RGBA);
	pauseMenuTexture.setWrapS(GL_CLAMP_TO_EDGE);
	pauseMenuTexture.setWrapT(GL_CLAMP_TO_EDGE);
	pauseMenuTexture.setMinFilter(GL_NEAREST);
	pauseMenuTexture.setMagFilter(GL_NEAREST);
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
 player->setLives(remainingLives);
	for (int i = 0; i < 1; ++i)
	{
		cout << "Initializing enemy " << i << endl;
		if (currentLevelNum == 0)	break;
		Enemies.push_back(new DonaldEnemy());
		Enemies[i]->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
		Enemies[i]->setTileMap(map);
	}
    bool shouldPlayDoorExitAnimation = false;
    bool shouldKeepSpawnDoorOpen = false;
 glm::ivec2 spawnTile(INIT_PLAYER_X_TILES, INIT_PLAYER_Y_TILES);
  if(currentLevelNum == 4)
		spawnTile = glm::ivec2(4, 1);
  else if(currentLevelNum == 5)
		spawnTile = glm::ivec2(15, 8);
    if(spawnAtDoorInLoadedLevel && !doorPositions.empty())
	{
		spawnTile = doorPositions[0];
       shouldPlayDoorExitAnimation = true;
	}
    spawnAtDoorInLoadedLevel = false;
	if(hasSpawnOverride)
	{
		spawnTile = spawnTileOverride;
		const int maxX = int(map->getMapSize().x) - 1;
		const int maxY = int(map->getMapSize().y) - 1;
		if(spawnTile.x < 0) spawnTile.x = 0;
		if(spawnTile.y < 0) spawnTile.y = 0;
		if(spawnTile.x > maxX) spawnTile.x = maxX;
		if(spawnTile.y > maxY) spawnTile.y = maxY;
       shouldPlayDoorExitAnimation = true;
       shouldKeepSpawnDoorOpen = true;
		hasSpawnOverride = false;
	}

	if(currentLevelNum == 0)
	{
		for(int i=0; i<int(doors.size()); ++i)
			doors[i]->open();
	}
    else
	{
		std::map<int, std::vector<glm::ivec2> >::const_iterator it = openedDoorsByLevel.find(currentLevelNum);
		if(it != openedDoorsByLevel.end())
		{
			for(int i=0; i<int(doors.size()); ++i)
			{
				if(containsDoorTile(it->second, doors[i]->getTilePos()))
					doors[i]->open();
			}
		}
	}
    if(shouldKeepSpawnDoorOpen)
	{
		for(int i=0; i<int(doors.size()); ++i)
		{
			if(doors[i]->getTilePos() == spawnTile)
			{
				doors[i]->open();
				break;
			}
		}
	}

	player->setPosition(glm::vec2(spawnTile.x * map->getTileSize(), spawnTile.y * map->getTileSize()));
	player->setTileMap(map);
    if(shouldPlayDoorExitAnimation)
		player->startDoorExitAnimation();
	projection = glm::ortho(0.f, float(map->getMapSize().x * map->getTileSize()), float(map->getMapSize().y * map->getTileSize()), 0.f);
	//projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
	currentTime = 0.0f;
	dWasPressed = false;
	hasDoorTarget = false;
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
    bool pPressed = Game::instance().getKey(GLFW_KEY_P);
	bool hPressed = Game::instance().getKey(GLFW_KEY_H);
	if(pPressed && !pWasPressed && !gameOverActive && !playerDeathActive)
		pauseActive = !pauseActive;

	if(pauseActive)
	{
		if(hPressed && !hWasPressed)
		{
			Game::instance().changeState(STATE_INSTRUCTIONS);
		}
		pWasPressed = pPressed;
		hWasPressed = hPressed;
		return;
	}
	pWasPressed = pPressed;
	hWasPressed = hPressed;

	bool dPressed = Game::instance().getKey(GLFW_KEY_D);
	if (player->isDoorInteractionStarted())	Enemy::Freeze();
	else	Enemy::Unfreeze();
	if(dPressed && !dWasPressed)
	{
		for(int i=0; i<int(doors.size()); ++i)
			doors[i]->toggleOpen();
	}
	dWasPressed = dPressed;

	for(int i=0; i<int(doors.size()); ++i)
		doors[i]->update(deltaTime);

	if(gameOverActive)
	{
		gameOverTimerMs -= deltaTime;
		if(gameOverTimerMs <= 0)
		{
			gameOverActive = false;
			remainingLives = MAX_LIVES;
			Game::instance().changeState(STATE_MENU);
		}
		return;
	}

	for(int i = 0; i < int(enemyExplosions.size()); )
	{
		enemyExplosions[i].timerMs -= deltaTime;
		if(enemyExplosions[i].timerMs <= 0)
			enemyExplosions.erase(enemyExplosions.begin() + i);
		else
			++i;
	}

	if(playerDeathActive)
	{
        player->update(deltaTime);
		if(player->isDeathAnimationFinished())
     {
			--remainingLives;
			if(remainingLives <= 0)
			{
             gameOverActive = true;
				gameOverTimerMs = GAME_OVER_DURATION_MS;
			}
			else
				loadLevel(currentLevelNum);
		}
		return;
	}

  player->update(deltaTime);
	bool leftPressed = Game::instance().getKey(GLFW_KEY_LEFT);
	bool rightPressed = Game::instance().getKey(GLFW_KEY_RIGHT);
    bool pushInputPressed = leftPressed || rightPressed;
    bool spacePressed = Game::instance().getKey(GLFW_KEY_SPACE);
	bool dropPressed = spacePressed && !spaceWasPressed;
	for(int i=0; i<int(weights.size()); ++i)
	{
      if(i >= int(weightPushLatch.size()))
			weightPushLatch.push_back(false);

		weights[i]->update(deltaTime);
        bool collidingWithPlayer = false;
		if(weights[i]->isOnGround())
		{
           glm::vec2 playerPos = player->getPosition();
			glm::ivec2 playerSize = player->getSize();
			collidingWithPlayer = collidesWith(playerPos, playerSize, weights[i]->getPosition(), weights[i]->getSize());
			if(collidingWithPlayer && pushInputPressed && !weightPushLatch[i])
			{
                float playerCenter = playerPos.x + playerSize.x / 2.f;
				float weightCenter = weights[i]->getPosition().x + weights[i]->getSize().x / 2.f;
				if(rightPressed && playerCenter < weightCenter)
					weights[i]->push(1);
				else if(leftPressed && playerCenter > weightCenter)
					weights[i]->push(-1);
			}
		}
      if(!pushInputPressed || !collidingWithPlayer)
			weightPushLatch[i] = false;
		else
			weightPushLatch[i] = true;
	}
	for(int i=0; i<int(bombs.size()); ++i)
		bombs[i]->update(deltaTime);

	glm::vec2 playerPos = player->getPosition();
	glm::ivec2 playerSize = player->getSize();

    if(dropPressed)
	{
      int bombToDrop = -1;
		for(int i=0; i<int(bombs.size()); ++i)
		{
			if(bombs[i]->isCollected())
			{
				bombToDrop = i;
				break;
			}
		}
		if(bombToDrop >= 0)
		{
			glm::ivec2 bombSize = bombs[bombToDrop]->getSize();
			glm::ivec2 dropPos(
				int(playerPos.x + (playerSize.x - bombSize.x) * 0.5f),
				int(playerPos.y));
			bombs[bombToDrop]->dropAt(dropPos);
		}
	}

	for(int i=0; i<int(bombs.size()); ++i)
	{
		if(bombs[i]->canBeCollected() && collidesWith(playerPos, playerSize, bombs[i]->getPosition(), bombs[i]->getSize()))
			bombs[i]->collect();
	}
	spaceWasPressed = spacePressed;

	glm::ivec2 playerTilePos((int(playerPos.x) + 8) / map->getTileSize(),(int(playerPos.y) + 15) / map->getTileSize());
	for (int i = 0; i < Enemies.size(); ++i)	
	{
		Enemies[i]->update(deltaTime, playerPos);
	}
	for (int i = 0; i < int(Enemies.size()); )
	{
		bool killedByWeight = false;
		int bombHitIndex = -1;
		glm::vec2 enemyPos = Enemies[i]->getPosition();
		glm::ivec2 enemySize = Enemies[i]->getCollisionSize();
		for(int j = 0; j < int(weights.size()) && !killedByWeight; ++j)
		{
			if(!weights[j]->isMoving())
				continue;

			if(collidesWith(enemyPos, enemySize, weights[j]->getPosition(), weights[j]->getSize()))
				killedByWeight = true;
		}
      for(int j=0; j<int(bombs.size()); ++j)
		{
            if(!bombs[j]->isArmed() || bombs[j]->isCollected())
				continue;

         if(collidesWith(enemyPos, enemySize, bombs[j]->getPosition(), bombs[j]->getSize()))
			{
				killedByWeight = true;
               bombHitIndex = j;
				break;
			}
		}

		if(killedByWeight)
		{
          EnemyExplosion explosion;
			explosion.pos = enemyPos;
			explosion.timerMs = EXPLOSION_DURATION_MS;
			enemyExplosions.push_back(explosion);
          if(bombHitIndex >= 0)
			{
				delete bombs[bombHitIndex];
				bombs.erase(bombs.begin() + bombHitIndex);
			}
			delete Enemies[i];
			Enemies.erase(Enemies.begin() + i);
		}
		else
			++i;
	}

	if(!godMode && !playerDeathActive)
	{
		glm::vec2 playerPosNow = player->getPosition();
		glm::ivec2 playerSizeNow = player->getSize();
		for(int i = 0; i < int(Enemies.size()); ++i)
		{
			if(collidesWith(playerPosNow, playerSizeNow, Enemies[i]->getPosition(), Enemies[i]->getCollisionSize()))
			{
				playerDeathActive = true;
             player->startDeathAnimation();
				break;
			}
		}
	}
	for (int i = 0; i < int(keys.size()); )
	{
		if (keys[i]->getTilePos() == playerTilePos)
		{
			player->addKey();
			map->removeKeyAtTile(keys[i]->getTilePos());
			delete keys[i];
			keys.erase(keys.begin() + i);
			if (currentLevelNum == 0 && hasReturnPoint)
			{
				if (!containsDoorTile(collectedRoomKeys[returnLevelNum], returnTilePos))
					collectedRoomKeys[returnLevelNum].push_back(returnTilePos);
			}
		}
		else
			++i;
	}

	if(player->isTubeTraveling())
	{
		glm::vec2 playerPos = player->getPosition();
      glm::ivec2 playerPosI(int(playerPos.x), int(playerPos.y));
		bool onTop = map->isTubeTile(playerPosI, true);
		bool onBottom = map->isTubeTile(playerPosI, false);
		bool onBottomAbove = map->isTubeTile(glm::ivec2(playerPosI.x, playerPosI.y - 16), false);

      glm::ivec2 sourceTile = map->worldToTile(playerPos + glm::vec2(float(PLAYER_CENTER_OFFSET_X_PX), float(PLAYER_FOOT_OFFSET_Y_PX)));
		glm::ivec2 exitTile = sourceTile;

		auto trySourceOffset = [&](int offsetY) -> bool
		{
			glm::ivec2 candidate = map->worldToTile(playerPos + glm::vec2(float(PLAYER_CENTER_OFFSET_X_PX), float(offsetY)));
			glm::ivec2 candidateExit = map->getTubeExit(candidate);
			if(candidateExit != candidate)
			{
				sourceTile = candidate;
				exitTile = candidateExit;
				return true;
			}
			return false;
		};

		if(onBottomAbove)
		{
			if(!trySourceOffset(PLAYER_HEAD_OFFSET_Y_PX))
				if(!trySourceOffset(PLAYER_HEAD_OFFSET_Y_MINUS_ONE_PX))
					trySourceOffset(PLAYER_FOOT_OFFSET_Y_MINUS_ONE_PX);
		}
		else
		{
			if(!trySourceOffset(PLAYER_FOOT_OFFSET_Y_PX))
				if(!trySourceOffset(PLAYER_FOOT_OFFSET_Y_MINUS_ONE_PX))
					trySourceOffset(PLAYER_HEAD_OFFSET_Y_PX);
		}

		cout << "[TubeDebug] Scene travel: playerPos=(" << playerPos.x << "," << playerPos.y << ") onTop=" << onTop << " onBottom=" << onBottom << " onBottomAbove=" << onBottomAbove << " sourceTile=(" << sourceTile.x << "," << sourceTile.y << ") exitTile=(" << exitTile.x << "," << exitTile.y << ")" << endl;
		glm::vec2 exitWorld = map->tileToWorld(exitTile);
       bool exitFromTop = map->isTubeBottomTile(exitTile);
       glm::vec2 exitFinalWorld;
		if(exitFromTop)
			exitFinalWorld = exitWorld + glm::vec2(0.f, float(PLAYER_COLLISION_HEIGHT_PX));
		else
			exitFinalWorld = exitWorld - glm::vec2(0.f, float(PLAYER_COLLISION_HEIGHT_PX));
		player->setTubeExitPos(glm::ivec2(int(exitFinalWorld.x), int(exitFinalWorld.y)));
        player->setTubeExitFromTop(exitFromTop);
		player->startTubeExit();
      if(exitFromTop)
            player->setPosition(exitFinalWorld - glm::vec2(0.f, float(PLAYER_COLLISION_HEIGHT_PX)));
		else
            player->setPosition(exitFinalWorld + glm::vec2(0.f, float(PLAYER_COLLISION_HEIGHT_PX)));
	}

	if(player->isTubeDone())
		player->resetTubeState();

	if(player->isDoorInteractionStarted())
	{
		glm::vec2 playerPos = player->getPosition();
		glm::ivec2 playerTilePos(
			(int(playerPos.x) + 8) / map->getTileSize(),
			(int(playerPos.y) + 15) / map->getTileSize());

		int doorIdx = findClosestDoorIndex(playerTilePos);
		if(doorIdx >= 0)
		{
			doors[doorIdx]->open();
           if(currentLevelNum != 0)
			{
				std::vector<glm::ivec2> &openedDoors = openedDoorsByLevel[currentLevelNum];
				const glm::ivec2 openedDoorPos = doors[doorIdx]->getTilePos();
				if(!containsDoorTile(openedDoors, openedDoorPos))
					openedDoors.push_back(openedDoorPos);
			}
			hasDoorTarget = true;
			doorTargetTilePos = doors[doorIdx]->getTilePos();
           if(currentLevelNum != 0)
			{
				hasReturnPoint = true;
				returnLevelNum = currentLevelNum;
				returnTilePos = doorTargetTilePos;
			}
		}
	}

	if(player->hasDoorTransitionEnded())
	{
		player->resetDoorState();

		if(currentLevelNum == 0)
		{
			// Returning from key room: restore suspended level, do NOT reload.
			if(hasReturnPoint)
			{
				restoreSuspendedLevelFromKeyRoom();
				return;
			}
		}
		else
		{
			// Entering key room from normal level: suspend level first.
			suspendCurrentLevelForKeyRoom();
			spawnAtDoorInLoadedLevel = hasDoorTarget;
			loadLevel(0);
			return;
		}
	}
}

void Scene::render()
{
	glm::mat4 modelview;

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
    float sceneDarken = 1.f;
	if(gameOverActive)
		sceneDarken = GAME_OVER_DARKEN_FACTOR;
	else if(pauseActive)
		sceneDarken = PAUSE_MENU_DARKEN_FACTOR;
	texProgram.setUniform4f("color", sceneDarken, sceneDarken, sceneDarken, 1.0f);
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	map->render();
   for(int i=0; i<int(doors.size()); ++i)
		doors[i]->render();
   for(int i=0; i<int(keys.size()); ++i)
		keys[i]->render();
   for(int i=0; i<int(weights.size()); ++i)
		weights[i]->render();
    for(int i=0; i<int(bombs.size()); ++i)
		bombs[i]->render();
   player->render();
    if(bombHudVao != 0)
	{
		int carriedBombs = 0;
		for(int i = 0; i < int(bombs.size()); ++i)
		{
			if(bombs[i]->isCollected())
				++carriedBombs;
		}

     const float startX = 0.f;
		for(int i = 0; i < carriedBombs; ++i)
		{
         glm::mat4 bombHudModelview = glm::translate(glm::mat4(1.0f), glm::vec3(startX + float(i * HUD_ICON_SPACING_PX), float(HUD_BOMB_ROW_Y_PX), 0.f));
			texProgram.setUniformMatrix4f("modelview", bombHudModelview);
			texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
			glEnable(GL_TEXTURE_2D);
			bombHudTexture.use();
			glBindVertexArray(bombHudVao);
			glEnableVertexAttribArray(bombHudPosLocation);
			glEnableVertexAttribArray(bombHudTexCoordLocation);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glDisable(GL_TEXTURE_2D);
		}
	}
    texProgram.setUniform4f("color", sceneDarken, sceneDarken, sceneDarken, 1.0f);
   for (int i = 0; i < int(Enemies.size()); ++i)
		Enemies[i]->render();
    texProgram.setUniform4f("color", sceneDarken, sceneDarken, sceneDarken, 1.0f);
   if(explosionVao != 0)
	{
		const float frameWidthUv = 1.f / float(EXPLOSION_FRAME_COUNT);
		for(int i = 0; i < int(enemyExplosions.size()); ++i)
		{
			const int elapsedMs = EXPLOSION_DURATION_MS - enemyExplosions[i].timerMs;
			int frame = (elapsedMs * EXPLOSION_FRAME_COUNT) / EXPLOSION_DURATION_MS;
			if(frame < 0) frame = 0;
			if(frame >= EXPLOSION_FRAME_COUNT) frame = EXPLOSION_FRAME_COUNT - 1;

			glm::mat4 explosionModelview = glm::translate(glm::mat4(1.0f), glm::vec3(enemyExplosions[i].pos.x, enemyExplosions[i].pos.y, 0.f));
			texProgram.setUniformMatrix4f("modelview", explosionModelview);
			texProgram.setUniform2f("texCoordDispl", frame * frameWidthUv, 0.f);
			glEnable(GL_TEXTURE_2D);
			explosionTexture.use();
			glBindVertexArray(explosionVao);
			glEnableVertexAttribArray(explosionPosLocation);
			glEnableVertexAttribArray(explosionTexCoordLocation);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glDisable(GL_TEXTURE_2D);
		}
   if(pauseActive && gameOverVao != 0)
	{
		texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
		glm::mat4 pauseModelview = glm::mat4(1.0f);
		texProgram.setUniformMatrix4f("modelview", pauseModelview);
		texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
		glEnable(GL_TEXTURE_2D);
		pauseMenuTexture.use();
		glBindVertexArray(gameOverVao);
		glEnableVertexAttribArray(gameOverPosLocation);
		glEnableVertexAttribArray(gameOverTexCoordLocation);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glDisable(GL_TEXTURE_2D);
	}
	}
   if(gameOverActive && gameOverVao != 0)
	{
       int elapsedMs = GAME_OVER_DURATION_MS - gameOverTimerMs;
		if(elapsedMs < 0) elapsedMs = 0;
		float appearAlpha = float(elapsedMs) / float(GAME_OVER_APPEAR_TIME_MS);
		if(appearAlpha > 1.f) appearAlpha = 1.f;

		texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, appearAlpha);
		glm::mat4 gameOverModelview = glm::mat4(1.0f);
		texProgram.setUniformMatrix4f("modelview", gameOverModelview);
		texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
		glEnable(GL_TEXTURE_2D);
		gameOverTexture.use();
		glBindVertexArray(gameOverVao);
		glEnableVertexAttribArray(gameOverPosLocation);
		glEnableVertexAttribArray(gameOverTexCoordLocation);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glDisable(GL_TEXTURE_2D);
	}
}

void Scene::freeDoors()
{
	for(int i=0; i<int(doors.size()); ++i)
		delete doors[i];
	doors.clear();
}

void Scene::freeKeys()
{
	for(int i=0; i<int(keys.size()); ++i)
		delete keys[i];
	keys.clear();
}

void Scene::freeEnemies()
{
	for(int i=0; i<int(Enemies.size()); ++i)
		delete Enemies[i];
	Enemies.clear();
}

void Scene::freeBombs()
{
	for(int i=0; i<int(bombs.size()); ++i)
		delete bombs[i];
	bombs.clear();
}

void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if(!texProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << texProgram.log() << endl << endl;
	}
	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}

void Scene::toggleGodMode()
{
	godMode = !godMode;
}

void Scene::giveAllKeys()
{
}

void Scene::resetForNewGame()
{
	remainingLives = MAX_LIVES;
 gameOverActive = false;
	gameOverTimerMs = 0;
 pauseActive = false;
	pWasPressed = false;
	hWasPressed = false;
	openedDoorsByLevel.clear();
	collectedRoomKeys.clear();
	hasReturnPoint = false;
	hasSpawnOverride = false;
	hasDoorTarget = false;
	spawnAtDoorInLoadedLevel = false;
}

void Scene::restartCurrentLevel()
{
	const int levelToReload = (currentLevelNum < 0) ? 1 : currentLevelNum;
	resetForNewGame();
	clearSuspendedLevel();
	loadLevel(levelToReload);
}

void Scene::loadLevel(int levelNum)
{
    if(levelNum != currentLevelNum && levelNum >= 0)
		remainingLives = MAX_LIVES;

	// Normal transitions should respawn, so discard any suspended snapshot.
	if(levelNum != 0)
		clearSuspendedLevel();

 if(map != NULL)
	{
		delete map;
		map = NULL;
	}
	if(player != NULL)
	{
		delete player;
		player = NULL;
	}
	for(int i=0; i<int(weights.size()); ++i)
		delete weights[i];
	weights.clear();
  freeBombs();
	playerDeathActive = false;
    gameOverActive = false;
	gameOverTimerMs = 0;
	enemyExplosions.clear();

	if(levelNum == 0)
	{
     currentLevelNum = 0;
	 if (hasReturnPoint && containsDoorTile(collectedRoomKeys[returnLevelNum], returnTilePos))	init("levels/KeyRoom_collected.txt");
	 else init("levels/KeyRoom.txt");
		return;
	}
	if(levelNum < 1)
		return;

	currentLevelNum = levelNum;

	stringstream levelPath;
	levelPath << "levels/level";
	if(levelNum < 10)
		levelPath << '0';
	levelPath << levelNum << ".txt";

	init(levelPath.str());
}

void Scene::loadLevelFile(const std::string &levelPath)
{
	if(map != NULL)
	{
		delete map;
		map = NULL;
	}
	if(player != NULL)
	{
		delete player;
		player = NULL;
	}
  freeBombs();
	playerDeathActive = false;
    gameOverActive = false;
	gameOverTimerMs = 0;
	enemyExplosions.clear();

	init(levelPath);
}

int Scene::findClosestDoorIndex(const glm::ivec2 &playerTilePos) const
{
	int closestIdx = -1;
	int closestDist = std::numeric_limits<int>::max();
	for(int i=0; i<int(doors.size()); ++i)
	{
		glm::ivec2 doorTile = doors[i]->getTilePos();
		int dist = abs(doorTile.x - playerTilePos.x) + abs(doorTile.y - playerTilePos.y);
		if(dist < closestDist)
		{
			closestDist = dist;
			closestIdx = i;
		}
	}

	return closestIdx;
}

bool Scene::isGodMode() const
{
	return godMode;
}

void Scene::clearSuspendedLevel()
{
	if(suspendedMap != NULL)
	{
		delete suspendedMap;
		suspendedMap = NULL;
	}
	if(suspendedPlayer != NULL)
	{
		delete suspendedPlayer;
		suspendedPlayer = NULL;
	}

	for(int i=0; i<int(suspendedDoors.size()); ++i) delete suspendedDoors[i];
	for(int i=0; i<int(suspendedKeys.size()); ++i) delete suspendedKeys[i];
	for(int i=0; i<int(suspendedWeights.size()); ++i) delete suspendedWeights[i];
	for(int i=0; i<int(suspendedBombs.size()); ++i) delete suspendedBombs[i];
	for(int i=0; i<int(suspendedEnemies.size()); ++i) delete suspendedEnemies[i];

	suspendedDoors.clear();
	suspendedKeys.clear();
	suspendedWeights.clear();
	suspendedBombs.clear();
	suspendedEnemies.clear();
	suspendedWeightPushLatch.clear();

	hasSuspendedLevel = false;
	suspendedLevelNum = -1;
}

void Scene::suspendCurrentLevelForKeyRoom()
{
	if(hasSuspendedLevel)
		return;

	hasSuspendedLevel = true;
	suspendedLevelNum = currentLevelNum;

	suspendedMap = map;           map = NULL;
	suspendedPlayer = player;     player = NULL;

	suspendedDoors.swap(doors);
	suspendedKeys.swap(keys);
	suspendedWeights.swap(weights);
	suspendedBombs.swap(bombs);
	suspendedEnemies.swap(Enemies);
	suspendedWeightPushLatch.swap(weightPushLatch);
}

void Scene::restoreSuspendedLevelFromKeyRoom()
{
	if(!hasSuspendedLevel)
		return;

	// Destroy current key-room objects
	if(map != NULL) { delete map; map = NULL; }
	if(player != NULL) { delete player; player = NULL; }

	freeDoors();
	freeKeys();
	freeEnemies();
	for(int i=0; i<int(weights.size()); ++i) delete weights[i];
	weights.clear();
	freeBombs();

	// Restore suspended level
	map = suspendedMap;               suspendedMap = NULL;
	player = suspendedPlayer;         suspendedPlayer = NULL;
	doors.swap(suspendedDoors);
	keys.swap(suspendedKeys);
	weights.swap(suspendedWeights);
	bombs.swap(suspendedBombs);
	Enemies.swap(suspendedEnemies);
	weightPushLatch.swap(suspendedWeightPushLatch);

	currentLevelNum = suspendedLevelNum;

	if(player != NULL && map != NULL)
	{
		player->setTileMap(map);
		player->setPosition(glm::vec2(
			float(returnTilePos.x * map->getTileSize()),
			float(returnTilePos.y * map->getTileSize())));
		player->startDoorExitAnimation();

		projection = glm::ortho(
			0.f,
			float(map->getMapSize().x * map->getTileSize()),
			float(map->getMapSize().y * map->getTileSize()),
			0.f);
	}

	Enemy::Unfreeze();
	hasSuspendedLevel = false;
	suspendedLevelNum = -1;
}

