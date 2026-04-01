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
   const int CLOCK_FREEZE_DURATION_MS = 3000;
    const int LEVEL_COMPLETED_DURATION_MS = 5000;
   const int NEXT_LEVEL_BLINK_INTERVAL_MS = 500;
   const int LEVEL_COMPLETE_STARS_COUNT = 3;
	const int LEVEL_COMPLETE_STARS_SIZE_PX = 32;
	const int LEVEL_COMPLETE_STARS_ANIM_FPS = 6;
 const int LEVEL_COMPLETE_STARS_DEFAULT_BOTTOM_MARGIN_PX = 36;
	const int LEVEL_COMPLETE_STARS_DEFAULT_SPACING_PX = 72;

	struct LevelCompleteStarsLayout
	{
		int centerXOffsetPx;
		int bottomMarginPx;
		int spacingPx;
	};

	LevelCompleteStarsLayout getLevelCompleteStarsLayout(int levelNum)
	{
		LevelCompleteStarsLayout layout;
		layout.centerXOffsetPx = 0;
		layout.bottomMarginPx = LEVEL_COMPLETE_STARS_DEFAULT_BOTTOM_MARGIN_PX;
		layout.spacingPx = LEVEL_COMPLETE_STARS_DEFAULT_SPACING_PX;

		if(levelNum == 4)
		{
			layout.bottomMarginPx = 52;
		}
		else if(levelNum == 5)
		{
			layout.bottomMarginPx = 60;
		}

		return layout;
	}
  const int GAME_OVER_DURATION_MS = 5000;
	const int GAME_OVER_APPEAR_TIME_MS = 700;
	const float GAME_OVER_DARKEN_FACTOR = 0.45f;
  const float LEVEL_COMPLETED_DARKEN_FACTOR = 0.25f;
    const int SHIELD_HIT_INVULN_TIME_MS = 600;
   const float PAUSE_MENU_DARKEN_FACTOR = 0.45f;
	const int HUD_BOMB_ICON_SIZE_PX = 16;
	const int HUD_ICON_SPACING_PX = 15;
   const int HUD_KEY_ROW_Y_PX = 16;
   const int HUD_KEY_COUNTER_Y_PX = 20;
	const int HUD_BOMB_ROW_Y_PX = 32;
  const int HUD_KEY_COUNTER_START_X_PX = 25;
	const int HUD_KEY_DIGIT_TILE_COUNT = 10;
	const int HUD_KEY_DIGIT_WIDTH_PX = 11;
   const int ENEMY_BULLET_SIZE_PX = 16;
	const int ENEMY_BULLET_SPEED_PX = 2;
	const int ENEMY_BULLET_SPAWN_OFFSET_X_PX = 8;
   // Franco sprite is 32px tall and rendered 16px above its collision box.
	// Using enemy world Y (collision top) places bullet at sprite half-height.
	const int ENEMY_BULLET_SPAWN_OFFSET_Y_PX = 12;
    const int LEVEL02_PIOLIN_MIN_COL = 4;
	const int LEVEL02_PIOLIN_MAX_COL = 14;
	const int LEVEL05_FRANCO_MIN_COL = 2;
	const int LEVEL05_FRANCO_MAX_COL = 13;
	const int ITEM_ROOM_KEY = 0;
	const int ITEM_ROOM_SHIELD = 1;
	const int ITEM_ROOM_BOMB = 2;
	const int ITEM_ROOM_EMPTY_KEY = 3;

	int getItemRoomTypeForDoor(int levelNum, const glm::ivec2 &doorTilePos)
	{
		if(levelNum == 2)
		{
           // Level 2 (row,col): (7,3)->bomb, (5,8)->shield, (5,15)->empty.
			if(doorTilePos == glm::ivec2(3, 7))
				return ITEM_ROOM_BOMB;
			if(doorTilePos == glm::ivec2(8, 5))
				return ITEM_ROOM_SHIELD;
           if(doorTilePos == glm::ivec2(15, 5))
				return ITEM_ROOM_EMPTY_KEY;
		}
		else if(levelNum == 3)
		{
          // Level 3 (row,col): (1,13)->empty, (5,9)->shield, (7,14)->bomb.
			if(doorTilePos == glm::ivec2(13, 1))
				return ITEM_ROOM_EMPTY_KEY;
			if(doorTilePos == glm::ivec2(9, 5))
				return ITEM_ROOM_SHIELD;
            if(doorTilePos == glm::ivec2(14, 7))
				return ITEM_ROOM_BOMB;
		}
        else if(levelNum == 4)
		{
         // Level 4 (row,col): (9,14)->empty, (1,14)->empty, (1,7)->bomb.
			if(doorTilePos == glm::ivec2(7, 1))
				return ITEM_ROOM_BOMB;
			if(doorTilePos == glm::ivec2(14, 1))
				return ITEM_ROOM_EMPTY_KEY;
			if(doorTilePos == glm::ivec2(14, 9))
				return ITEM_ROOM_EMPTY_KEY;
		}
        else if(levelNum == 5)
		{
         // Level 5 (row,col): (6,6)->empty, (4,4)->shield, (8,13)->bomb.
			if(doorTilePos == glm::ivec2(6, 6))
				return ITEM_ROOM_EMPTY_KEY;
			if(doorTilePos == glm::ivec2(13, 8))
				return ITEM_ROOM_BOMB;
			if(doorTilePos == glm::ivec2(4, 4))
				return ITEM_ROOM_SHIELD;
		}

		return ITEM_ROOM_KEY;
	}

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
   doorTargetIsLockedExit = false;
	hasSpawnOverride = false;
  spawnAtDoorInLoadedLevel = false;
	doorTargetTilePos = glm::ivec2(0);
	spawnTileOverride = glm::ivec2(0);
    pendingItemRoomType = ITEM_ROOM_KEY;
    pendingCarryBombFromRoom = false;
	pendingCarryShieldFromRoom = false;
    carriedBombsBeforeRoom = 0;
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
   enemyBulletVao = 0;
	enemyBulletVbo = 0;
	enemyBulletPosLocation = -1;
	enemyBulletTexCoordLocation = -1;
  bombHudVao = 0;
	bombHudVbo = 0;
	bombHudPosLocation = -1;
	bombHudTexCoordLocation = -1;
  keyHudVao = 0;
	keyHudVbo = 0;
	keyHudPosLocation = -1;
	keyHudTexCoordLocation = -1;
  keyCounterVao = 0;
	keyCounterVbo = 0;
	keyCounterPosLocation = -1;
	keyCounterTexCoordLocation = -1;
	playerDeathActive = false;
 gameOverActive = false;
	gameOverTimerMs = 0;
   levelCompletedActive = false;
	levelCompletedTimerMs = 0;
    playerShieldHitInvulnTimerMs = 0;
	pauseActive = false;
	pWasPressed = false;
	hWasPressed = false;
	kWasPressed = false;
   enemyExplosions.clear();
  enemyBullets.clear();
   remainingLives = MAX_LIVES;
   spaceWasPressed = false;
  clockFreezeTimerMs = 0;
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
    freeShieldItems();
    freeClockItems();
   if(explosionVao != 0)
		glDeleteVertexArrays(1, &explosionVao);
	if(explosionVbo != 0)
		glDeleteBuffers(1, &explosionVbo);
 if(enemyBulletVao != 0)
		glDeleteVertexArrays(1, &enemyBulletVao);
	if(enemyBulletVbo != 0)
		glDeleteBuffers(1, &enemyBulletVbo);
   if(gameOverVao != 0)
		glDeleteVertexArrays(1, &gameOverVao);
	if(gameOverVbo != 0)
		glDeleteBuffers(1, &gameOverVbo);
  if(bombHudVao != 0)
		glDeleteVertexArrays(1, &bombHudVao);
	if(bombHudVbo != 0)
		glDeleteBuffers(1, &bombHudVbo);
  if(keyHudVao != 0)
		glDeleteVertexArrays(1, &keyHudVao);
	if(keyHudVbo != 0)
		glDeleteBuffers(1, &keyHudVbo);
 if(keyCounterVao != 0)
		glDeleteVertexArrays(1, &keyCounterVao);
	if(keyCounterVbo != 0)
		glDeleteBuffers(1, &keyCounterVbo);
   for(int i = 0; i < int(levelCompletedStars.size()); ++i)
		delete levelCompletedStars[i];
	levelCompletedStars.clear();
}


void Scene::init(const std::string &sceneName)
{
	initShaders();
	freeDoors();
	freeKeys();
	freeEnemies();
   freeBombs();
	freeShieldItems();
	freeClockItems();
	map = TileMap::createTileMap(sceneName, glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	setNeededKeys();
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
    const bool hasExplicitLockedExitDoor = map->hasLockedExitDoorObject();
	const std::unordered_set<glm::ivec2, TileMap::IVec2Hash> &keyPositions = map->getKeyPositions();
	for(int i=0; i<int(doorPositions.size()); ++i)
	{
		Door *door = new Door();
       Door::Type doorType = Door::Type::Normal;
		if(map->isLockedExitDoorObject(doorPositions[i]))
			doorType = Door::Type::LockedExit;
		else if(!hasExplicitLockedExitDoor && currentLevelNum != 0 && i == 0)
			doorType = Door::Type::LockedExit;
		door->init(doorPositions[i], doorType, texProgram);
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
    const std::vector<glm::ivec2> &shieldTiles = map->getShieldPositions();
	for(int i=0; i<int(shieldTiles.size()); ++i)
	{
		ShieldItem *shieldItem = new ShieldItem();
		shieldItem->init(shieldTiles[i], texProgram);
		shieldItems.push_back(shieldItem);
	}
    const std::vector<glm::ivec2> &clockTiles = map->getClockPositions();
	for(int i=0; i<int(clockTiles.size()); ++i)
	{
		ClockItem *clockItem = new ClockItem();
		clockItem->init(clockTiles[i], texProgram);
		clockItems.push_back(clockItem);
	}
  weightPushLatch.assign(weights.size(), false);
  playerDeathActive = false;
    gameOverActive = false;
	gameOverTimerMs = 0;
    levelCompletedActive = false;
	levelCompletedTimerMs = 0;
	enemyExplosions.clear();
    enemyBullets.clear();
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

	enemyBulletTexture.loadFromFile("images/bullet.png", TEXTURE_PIXEL_FORMAT_RGBA);
	enemyBulletTexture.setWrapS(GL_CLAMP_TO_EDGE);
	enemyBulletTexture.setWrapT(GL_CLAMP_TO_EDGE);
	enemyBulletTexture.setMinFilter(GL_NEAREST);
	enemyBulletTexture.setMagFilter(GL_NEAREST);

	if(enemyBulletVao != 0)
		glDeleteVertexArrays(1, &enemyBulletVao);
	if(enemyBulletVbo != 0)
		glDeleteBuffers(1, &enemyBulletVbo);

	float bulletVertices[16] = {
		0.f, 0.f, 0.f, 0.f,
		16.f, 0.f, 1.f, 0.f,
		16.f, 16.f, 1.f, 1.f,
		0.f, 16.f, 0.f, 1.f
	};

	glGenVertexArrays(1, &enemyBulletVao);
	glBindVertexArray(enemyBulletVao);
	glGenBuffers(1, &enemyBulletVbo);
	glBindBuffer(GL_ARRAY_BUFFER, enemyBulletVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bulletVertices), bulletVertices, GL_STATIC_DRAW);
	enemyBulletPosLocation = texProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	enemyBulletTexCoordLocation = texProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));

  bombHudTexture.loadFromFile("images/bomba.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bombHudTexture.setWrapS(GL_CLAMP_TO_EDGE);
	bombHudTexture.setWrapT(GL_CLAMP_TO_EDGE);
	bombHudTexture.setMinFilter(GL_NEAREST);
	bombHudTexture.setMagFilter(GL_NEAREST);

    keyCounterTexture.loadFromFile("images/numbers.png", TEXTURE_PIXEL_FORMAT_RGBA);
	keyCounterTexture.setWrapS(GL_CLAMP_TO_EDGE);
	keyCounterTexture.setWrapT(GL_CLAMP_TO_EDGE);
	keyCounterTexture.setMinFilter(GL_NEAREST);
	keyCounterTexture.setMagFilter(GL_NEAREST);

	hudKeyTexture.loadFromFile("images/hudkey.png", TEXTURE_PIXEL_FORMAT_RGBA);
	hudKeyTexture.setWrapS(GL_CLAMP_TO_EDGE);
	hudKeyTexture.setWrapT(GL_CLAMP_TO_EDGE);
	hudKeyTexture.setMinFilter(GL_NEAREST);
	hudKeyTexture.setMagFilter(GL_NEAREST);

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

	if(keyHudVao != 0)
		glDeleteVertexArrays(1, &keyHudVao);
	if(keyHudVbo != 0)
		glDeleteBuffers(1, &keyHudVbo);

	float keyHudVertices[16] = {
		0.f, 0.f, 0.f, 0.f,
		float(hudKeyTexture.width()), 0.f, 1.f, 0.f,
		float(hudKeyTexture.width()), float(hudKeyTexture.height()), 1.f, 1.f,
		0.f, float(hudKeyTexture.height()), 0.f, 1.f
	};

	glGenVertexArrays(1, &keyHudVao);
	glBindVertexArray(keyHudVao);
	glGenBuffers(1, &keyHudVbo);
	glBindBuffer(GL_ARRAY_BUFFER, keyHudVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(keyHudVertices), keyHudVertices, GL_STATIC_DRAW);
	keyHudPosLocation = texProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	keyHudTexCoordLocation = texProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));

	if(keyCounterVao != 0)
		glDeleteVertexArrays(1, &keyCounterVao);
	if(keyCounterVbo != 0)
		glDeleteBuffers(1, &keyCounterVbo);

	const float digitUvWidth = 1.f / float(HUD_KEY_DIGIT_TILE_COUNT);
	float keyCounterVertices[16] = {
		0.f, 0.f, 0.f, 0.f,
		float(HUD_KEY_DIGIT_WIDTH_PX), 0.f, digitUvWidth, 0.f,
		float(HUD_KEY_DIGIT_WIDTH_PX), float(keyCounterTexture.height()), digitUvWidth, 1.f,
		0.f, float(keyCounterTexture.height()), 0.f, 1.f
	};

	glGenVertexArrays(1, &keyCounterVao);
	glBindVertexArray(keyCounterVao);
	glGenBuffers(1, &keyCounterVbo);
	glBindBuffer(GL_ARRAY_BUFFER, keyCounterVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(keyCounterVertices), keyCounterVertices, GL_STATIC_DRAW);
	keyCounterPosLocation = texProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	keyCounterTexCoordLocation = texProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));

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

	levelCompletedTexture.loadFromFile("images/lvl-Completed.png", TEXTURE_PIXEL_FORMAT_RGBA);
	levelCompletedTexture.setWrapS(GL_CLAMP_TO_EDGE);
	levelCompletedTexture.setWrapT(GL_CLAMP_TO_EDGE);
	levelCompletedTexture.setMinFilter(GL_NEAREST);
	levelCompletedTexture.setMagFilter(GL_NEAREST);

	nextLevelTexture.loadFromFile("images/nxtlev.png", TEXTURE_PIXEL_FORMAT_RGBA);
	nextLevelTexture.setWrapS(GL_CLAMP_TO_EDGE);
	nextLevelTexture.setWrapT(GL_CLAMP_TO_EDGE);
	nextLevelTexture.setMinFilter(GL_NEAREST);
	nextLevelTexture.setMagFilter(GL_NEAREST);

	pauseMenuTexture.loadFromFile("images/Pause-Menu.png", TEXTURE_PIXEL_FORMAT_RGBA);
	pauseMenuTexture.setWrapS(GL_CLAMP_TO_EDGE);
	pauseMenuTexture.setWrapT(GL_CLAMP_TO_EDGE);
	pauseMenuTexture.setMinFilter(GL_NEAREST);
	pauseMenuTexture.setMagFilter(GL_NEAREST);

	starsTexture.loadFromFile("images/stars.png", TEXTURE_PIXEL_FORMAT_RGBA);
	starsTexture.setWrapS(GL_CLAMP_TO_EDGE);
	starsTexture.setWrapT(GL_CLAMP_TO_EDGE);
	starsTexture.setMinFilter(GL_NEAREST);
	starsTexture.setMagFilter(GL_NEAREST);
	for(int i = 0; i < int(levelCompletedStars.size()); ++i)
		delete levelCompletedStars[i];
	levelCompletedStars.clear();

    const LevelCompleteStarsLayout starsLayout = getLevelCompleteStarsLayout(currentLevelNum);
	const float starsCenterX = (mapPixelW * 0.5f) + float(starsLayout.centerXOffsetPx);
	const float starsY = mapPixelH - float(starsLayout.bottomMarginPx + LEVEL_COMPLETE_STARS_SIZE_PX);
	for(int i = 0; i < LEVEL_COMPLETE_STARS_COUNT; ++i)
	{
		Sprite *star = Sprite::createSprite(
			glm::ivec2(LEVEL_COMPLETE_STARS_SIZE_PX, LEVEL_COMPLETE_STARS_SIZE_PX),
			glm::vec2(1.f / 3.f, 1.f),
			&starsTexture,
			&texProgram);
		star->setNumberAnimations(1);
		star->setAnimationSpeed(0, LEVEL_COMPLETE_STARS_ANIM_FPS);
		star->addKeyframe(0, glm::vec2(0.f / 3.f, 0.f));
		star->addKeyframe(0, glm::vec2(1.f / 3.f, 0.f));
		star->addKeyframe(0, glm::vec2(2.f / 3.f, 0.f));
		star->changeAnimation(0);

     const int centeredIndex = i - (LEVEL_COMPLETE_STARS_COUNT / 2);
		const float starX = starsCenterX + float(centeredIndex * starsLayout.spacingPx) - (LEVEL_COMPLETE_STARS_SIZE_PX * 0.5f);
		star->setPosition(glm::vec2(starX, starsY));
		levelCompletedStars.push_back(star);
	}
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
 player->setLives(remainingLives);
 player->setGodModeShieldVisual(godMode);
	for (int i = 0; i < 1; ++i)
	{
		cout << "Initializing enemy " << i << endl;
      if (currentLevelNum == 0)	break;
		if(currentLevelNum == 2 || currentLevelNum == 4)
			Enemies.push_back(new PiolinEnemy());
		else if(currentLevelNum >= 5)
			Enemies.push_back(new FrancoEnemy());
		else
			Enemies.push_back(new DonaldEnemy());
		Enemies[i]->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
		Enemies[i]->setTileMap(map);
       if(currentLevelNum == 2)
		{
			const glm::ivec2 enemySpawnTile(3, 5);
			Enemies[i]->setPosition(glm::vec2(enemySpawnTile.x * map->getTileSize(), enemySpawnTile.y * map->getTileSize()));
		}
       else if(currentLevelNum == 5)
		{
			const glm::ivec2 enemySpawnTile(2, 4);
			Enemies[i]->setPosition(glm::vec2(enemySpawnTile.x * map->getTileSize(), enemySpawnTile.y * map->getTileSize()));
		}
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
   doorTargetIsLockedExit = false;
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
    bool pPressed = Game::instance().getKey(GLFW_KEY_P);
	bool hPressed = Game::instance().getKey(GLFW_KEY_H);
	bool kPressed = Game::instance().getKey(GLFW_KEY_K);
	if (kPressed)	
	{
		kWasPressed = true;
		addKeys(neededKeys - totalKeys);
	}
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
	if(clockFreezeTimerMs > 0)
	{
		clockFreezeTimerMs -= deltaTime;
		if(clockFreezeTimerMs < 0)
			clockFreezeTimerMs = 0;
	}
	if(player->isDoorInteractionStarted() || clockFreezeTimerMs > 0)
		Enemy::Freeze();
	else
		Enemy::Unfreeze();
    if(dPressed && !dWasPressed)
	{
      if(keys.empty())
		{
			for(int i=0; i<int(doors.size()); ++i)
             if(!doors[i]->isLockedExit())
					doors[i]->toggleOpen();
		}
	}
	dWasPressed = dPressed;

	for(int i=0; i<int(doors.size()); ++i)
		doors[i]->update(deltaTime);

	if(levelCompletedActive)
	{
     for(int i = 0; i < int(levelCompletedStars.size()); ++i)
			levelCompletedStars[i]->update(deltaTime);

		levelCompletedTimerMs -= deltaTime;
		if(levelCompletedTimerMs <= 0)
		{
			levelCompletedActive = false;
			int nextLevel = currentLevelNum + 1;
			if(nextLevel > 5)
               Game::instance().changeState(STATE_CREDITS);
			else
				loadLevel(nextLevel);
		}
		return;
	}

	if(playerShieldHitInvulnTimerMs > 0)
	{
		playerShieldHitInvulnTimerMs -= deltaTime;
		if(playerShieldHitInvulnTimerMs < 0)
			playerShieldHitInvulnTimerMs = 0;
	}

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
         {
				if(currentLevelNum > 0)
				{
					openedDoorsByLevel.erase(currentLevelNum);
					collectedRoomKeys.erase(currentLevelNum);
				}
				hasReturnPoint = false;
				hasDoorTarget = false;
				doorTargetIsLockedExit = false;
				hasSpawnOverride = false;
				spawnAtDoorInLoadedLevel = false;
				pendingItemRoomType = ITEM_ROOM_KEY;
				pendingCarryBombFromRoom = false;
				pendingCarryShieldFromRoom = false;
				carriedBombsBeforeRoom = 0;
				loadLevel(currentLevelNum);
           }
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
        {
			bombs[i]->collect();
			if(currentLevelNum == 0 && hasReturnPoint)
				pendingCarryBombFromRoom = true;
		}
	}
 for(int i=0; i<int(shieldItems.size()); ++i)
	{
		if(!shieldItems[i]->isCollected() && collidesWith(playerPos, playerSize, shieldItems[i]->getPosition(), shieldItems[i]->getSize()))
		{
			shieldItems[i]->collect();
			player->activateShield();
           if(currentLevelNum == 0 && hasReturnPoint)
				pendingCarryShieldFromRoom = true;
		}
	}
 for(int i=0; i<int(clockItems.size()); ++i)
	{
		if(!clockItems[i]->isCollected() && collidesWith(playerPos, playerSize, clockItems[i]->getPosition(), clockItems[i]->getSize()))
		{
			clockItems[i]->collect();
			clockFreezeTimerMs = CLOCK_FREEZE_DURATION_MS;
		}
	}
	spaceWasPressed = spacePressed;

	glm::ivec2 playerTilePos((int(playerPos.x) + 8) / map->getTileSize(),(int(playerPos.y) + 15) / map->getTileSize());
    auto getEnemyInteractionPos = [&](Enemy *enemy) -> glm::vec2
	{
		glm::vec2 pos = enemy->getPosition();
     if((currentLevelNum == 2 && enemy->getType() == Enemy::Type::PIOLIN) ||
		   (currentLevelNum == 5 && enemy->getType() == Enemy::Type::FRANCO))
		{
         // Constrained horizontal patrol uses a visual-alignment Y lock.
			// Shift interaction box one tile down so player/items contact matches
			// on-screen sprite contact without moving enemy rendering.
			pos.y += float(map->getTileSize());
		}
		return pos;
	};
	for (int i = 0; i < Enemies.size(); ++i)	
	{
		Enemies[i]->update(deltaTime, playerPos);
       if(currentLevelNum == 2 && Enemies[i]->getType() == Enemy::Type::PIOLIN)
		{
			const int tileSize = map->getTileSize();
			const float minX = float(LEVEL02_PIOLIN_MIN_COL * tileSize);
			const float maxX = float(LEVEL02_PIOLIN_MAX_COL * tileSize);
            PiolinEnemy *piolin = static_cast<PiolinEnemy*>(Enemies[i]);
			piolin->enforceHorizontalPatrolRange(minX, maxX);
		}
       else if(currentLevelNum == 5 && Enemies[i]->getType() == Enemy::Type::FRANCO)
		{
			const int tileSize = map->getTileSize();
			const float minX = float(LEVEL05_FRANCO_MIN_COL * tileSize);
			const float maxX = float(LEVEL05_FRANCO_MAX_COL * tileSize);
			FrancoEnemy *franco = static_cast<FrancoEnemy*>(Enemies[i]);
			franco->enforceHorizontalPatrolRange(minX, maxX);

			int shotDir = 0;
			if(franco->consumePendingShot(shotDir))
			{
				EnemyBullet bullet;
				const glm::vec2 enemyPos = getEnemyInteractionPos(Enemies[i]);
                const glm::vec2 enemyRawPos = Enemies[i]->getPosition();
				const glm::ivec2 enemySize = Enemies[i]->getCollisionSize();
				bullet.dirSign = shotDir;
				if(shotDir > 0)
                  bullet.pos = glm::vec2(enemyPos.x + float(enemySize.x + ENEMY_BULLET_SPAWN_OFFSET_X_PX), enemyRawPos.y + float(ENEMY_BULLET_SPAWN_OFFSET_Y_PX));
				else
                    bullet.pos = glm::vec2(enemyPos.x + float(-ENEMY_BULLET_SIZE_PX - ENEMY_BULLET_SPAWN_OFFSET_X_PX), enemyRawPos.y + float(ENEMY_BULLET_SPAWN_OFFSET_Y_PX));
				enemyBullets.push_back(bullet);
			}
		}
	}

	if(clockFreezeTimerMs == 0 && !player->isDoorInteractionStarted())
	{
		for(int i = 0; i < int(enemyBullets.size()); ++i)
			enemyBullets[i].pos.x += float(enemyBullets[i].dirSign * ENEMY_BULLET_SPEED_PX);
	}

	const float mapPixelW = map->getMapSize().x * map->getTileSize();
	for(int i = 0; i < int(enemyBullets.size()); )
	{
		if(enemyBullets[i].pos.x >= mapPixelW || (enemyBullets[i].pos.x + ENEMY_BULLET_SIZE_PX) <= 0.f)
			enemyBullets.erase(enemyBullets.begin() + i);
		else
			++i;
	}
	for (int i = 0; i < int(Enemies.size()); )
	{
		bool killedByWeight = false;
		int bombHitIndex = -1;
     glm::vec2 enemyPos = getEnemyInteractionPos(Enemies[i]);
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

  if(!godMode && !playerDeathActive && playerShieldHitInvulnTimerMs == 0)
	{
		glm::vec2 playerPosNow = player->getPosition();
		glm::ivec2 playerSizeNow = player->getSize();
       bool bulletHitHandled = false;
		for(int i = 0; i < int(enemyBullets.size()); ++i)
		{
			if(collidesWith(playerPosNow, playerSizeNow, enemyBullets[i].pos, glm::ivec2(ENEMY_BULLET_SIZE_PX, ENEMY_BULLET_SIZE_PX)))
			{
				enemyBullets.erase(enemyBullets.begin() + i);
				if(player->hasShield())
				{
					player->consumeShield();
					playerShieldHitInvulnTimerMs = SHIELD_HIT_INVULN_TIME_MS;
				}
				else
				{
					playerDeathActive = true;
					player->startDeathAnimation();
				}
              bulletHitHandled = true;
				break;
			}
		}

		if(!bulletHitHandled)
		{
           for(int i = 0; i < int(Enemies.size()); ++i)
			{
              if(collidesWith(playerPosNow, playerSizeNow, getEnemyInteractionPos(Enemies[i]), Enemies[i]->getCollisionSize()))
				{
                   if(player->hasShield())
					 {
						player->consumeShield();
						playerShieldHitInvulnTimerMs = SHIELD_HIT_INVULN_TIME_MS;
					}
					else
					{
						playerDeathActive = true;
						player->startDeathAnimation();
					}
					break;
				}
			}
		}
	}
	for (int i = 0; i < int(keys.size()); )
	{
		if (keys[i]->getTilePos() == playerTilePos || kWasPressed)
		{
			addKeys(1);
			map->removeKeyAtTile(keys[i]->getTilePos());
			delete keys[i];
			keys.erase(keys.begin() + i);
			if (currentLevelNum == 0 && hasReturnPoint || kWasPressed)
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
          if(doors[doorIdx]->isLockedExit() && !keys.empty())
			{
				player->resetDoorState();
				return;
			}

			doors[doorIdx]->open();
           if(currentLevelNum != 0)
			{
				std::vector<glm::ivec2> &openedDoors = openedDoorsByLevel[currentLevelNum];
				const glm::ivec2 openedDoorPos = doors[doorIdx]->getTilePos();
				if(!containsDoorTile(openedDoors, openedDoorPos))
					openedDoors.push_back(openedDoorPos);
			}
			hasDoorTarget = true;
           doorTargetIsLockedExit = doors[doorIdx]->isLockedExit();
			doorTargetTilePos = doors[doorIdx]->getTilePos();
            pendingItemRoomType = getItemRoomTypeForDoor(currentLevelNum, doorTargetTilePos);
         if(currentLevelNum != 0 && !doorTargetIsLockedExit)
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
            if(doorTargetIsLockedExit)
			{
				levelCompletedActive = (totalKeys == neededKeys);
				levelCompletedTimerMs = LEVEL_COMPLETED_DURATION_MS;
				return;
			}

            pendingCarryBombFromRoom = false;
			pendingCarryShieldFromRoom = false;
            spawnAtDoorInLoadedLevel = true;
			suspendCurrentLevelForKeyRoom();
			loadLevel(0);
			return;
		}
	}
}

void Scene::render()
{
    glm::mat4 modelview(1.0f);

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);

	float sceneDarken = 1.f;
	if(gameOverActive)
		sceneDarken = GAME_OVER_DARKEN_FACTOR;
	else if(levelCompletedActive)
     sceneDarken = LEVEL_COMPLETED_DARKEN_FACTOR;
	else if(pauseActive)
		sceneDarken = PAUSE_MENU_DARKEN_FACTOR;

	texProgram.setUniform4f("color", sceneDarken, sceneDarken, sceneDarken, 1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

	map->render();
	for(int i=0; i<int(doors.size()); ++i) doors[i]->render();
	for(int i=0; i<int(keys.size()); ++i) keys[i]->render();
	for(int i=0; i<int(weights.size()); ++i) weights[i]->render();
	for(int i=0; i<int(bombs.size()); ++i) bombs[i]->render();
	for(int i=0; i<int(clockItems.size()); ++i) clockItems[i]->render();
	for(int i=0; i<int(shieldItems.size()); ++i) shieldItems[i]->render();
	player->render();

	if(bombHudVao != 0)
	{
      int keyCount = getKeyCount();
		if(keyCount < 0) keyCount = 0;
		if(keyCount > 9) keyCount = 9;
		if(keyHudVao != 0)
		{
			glm::mat4 keyHudModelview = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, float(HUD_KEY_ROW_Y_PX), 0.f));
			texProgram.setUniformMatrix4f("modelview", keyHudModelview);
			texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
			glEnable(GL_TEXTURE_2D);
			hudKeyTexture.use();
			glBindVertexArray(keyHudVao);
			glEnableVertexAttribArray(keyHudPosLocation);
			glEnableVertexAttribArray(keyHudTexCoordLocation);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glDisable(GL_TEXTURE_2D);
		}

       if(keyCounterVao != 0)
		{
          const float digitUvWidth = 1.f / float(HUD_KEY_DIGIT_TILE_COUNT);
            glm::mat4 keyCounterModelview = glm::translate(glm::mat4(1.0f), glm::vec3(float(HUD_KEY_COUNTER_START_X_PX), float(HUD_KEY_COUNTER_Y_PX), 0.f));
			texProgram.setUniformMatrix4f("modelview", keyCounterModelview);
         texProgram.setUniform2f("texCoordDispl", float(keyCount) * digitUvWidth, 0.f);
			glEnable(GL_TEXTURE_2D);
			keyCounterTexture.use();
          glBindVertexArray(keyCounterVao);
			glEnableVertexAttribArray(keyCounterPosLocation);
			glEnableVertexAttribArray(keyCounterTexCoordLocation);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glDisable(GL_TEXTURE_2D);
		}

		int carriedBombs = 0;
		for(int i = 0; i < int(bombs.size()); ++i)
			if(bombs[i]->isCollected())
				++carriedBombs;

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
	for(int i = 0; i < int(Enemies.size()); ++i)
		Enemies[i]->render();
	if(enemyBulletVao != 0)
	{
		for(int i = 0; i < int(enemyBullets.size()); ++i)
		{
			glm::mat4 bulletModelview = glm::translate(glm::mat4(1.0f), glm::vec3(enemyBullets[i].pos.x, enemyBullets[i].pos.y, 0.f));
			texProgram.setUniformMatrix4f("modelview", bulletModelview);
			texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
			glEnable(GL_TEXTURE_2D);
			enemyBulletTexture.use();
			glBindVertexArray(enemyBulletVao);
			glEnableVertexAttribArray(enemyBulletPosLocation);
			glEnableVertexAttribArray(enemyBulletTexCoordLocation);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glDisable(GL_TEXTURE_2D);
		}
	}

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
	}

  auto renderFullScreenOverlay = [&](Texture &texture, float alpha)
	{
		texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, alpha);
		texProgram.setUniformMatrix4f("modelview", glm::mat4(1.0f));
		texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
		glEnable(GL_TEXTURE_2D);
		texture.use();
		glBindVertexArray(gameOverVao);
		glEnableVertexAttribArray(gameOverPosLocation);
		glEnableVertexAttribArray(gameOverTexCoordLocation);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glDisable(GL_TEXTURE_2D);
	};

	if(gameOverActive && gameOverVao != 0)
	{

		int elapsedMs = GAME_OVER_DURATION_MS - gameOverTimerMs;
		if(elapsedMs < 0) elapsedMs = 0;
		float appearAlpha = float(elapsedMs) / float(GAME_OVER_APPEAR_TIME_MS);
		if(appearAlpha > 1.f) appearAlpha = 1.f;
		renderFullScreenOverlay(gameOverTexture, appearAlpha);
	}

	if(levelCompletedActive && gameOverVao != 0)
	{
		renderFullScreenOverlay(levelCompletedTexture, 1.0f);
     const int blinkPhase = levelCompletedTimerMs / NEXT_LEVEL_BLINK_INTERVAL_MS;
		if((blinkPhase % 2) == 0)
			renderFullScreenOverlay(nextLevelTexture, 1.0f);

		for(int i = 0; i < int(levelCompletedStars.size()); ++i)
			levelCompletedStars[i]->render();
	}

	if(pauseActive && gameOverVao != 0)
	{
		renderFullScreenOverlay(pauseMenuTexture, 1.0f);
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

void Scene::freeShieldItems()
{
	for(int i=0; i<int(shieldItems.size()); ++i)
		delete shieldItems[i];
	shieldItems.clear();
}

void Scene::freeClockItems()
{
	for(int i=0; i<int(clockItems.size()); ++i)
		delete clockItems[i];
	clockItems.clear();
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
   if(player != NULL)
		player->setGodModeShieldVisual(godMode);
}

void Scene::giveAllKeys()
{
   if(map == NULL)
		return;

	const bool hadKeys = !keys.empty();
	for(int i = 0; i < int(keys.size()); ++i)
	{
		if(player != NULL)
			player->addKey();
		map->removeKeyAtTile(keys[i]->getTilePos());
		delete keys[i];
	}
	keys.clear();

	if(hadKeys && currentLevelNum == 0 && hasReturnPoint)
	{
		if(!containsDoorTile(collectedRoomKeys[returnLevelNum], returnTilePos))
		{
			collectedRoomKeys[returnLevelNum].push_back(returnTilePos);
		}
	}
}

void Scene::resetForNewGame()
{
	remainingLives = MAX_LIVES;
 gameOverActive = false;
	gameOverTimerMs = 0;
  levelCompletedActive = false;
	levelCompletedTimerMs = 0;
   clockFreezeTimerMs = 0;
 pauseActive = false;
	pWasPressed = false;
	hWasPressed = false;
	openedDoorsByLevel.clear();
	collectedRoomKeys.clear();
	hasReturnPoint = false;
	hasSpawnOverride = false;
	hasDoorTarget = false;
   doorTargetIsLockedExit = false;
	spawnAtDoorInLoadedLevel = false;
   pendingItemRoomType = ITEM_ROOM_KEY;
    pendingCarryBombFromRoom = false;
	pendingCarryShieldFromRoom = false;
    carriedBombsBeforeRoom = 0;
   enemyBullets.clear();
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
    // Keep player lives when entering item/key rooms (level 0).
	// Only reset lives on normal level-to-level transitions.
	if(levelNum != currentLevelNum && levelNum >= 1)
		remainingLives = MAX_LIVES;

	// Normal transitions should respawn, so discard any suspended snapshot.
	if(levelNum != 0)
		clearSuspendedLevel();
	if (levelNum != 0)	
	{
		setTotalKeys(0);
		kWasPressed = false;
	}
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
  freeShieldItems();
  freeClockItems();
	playerDeathActive = false;
    gameOverActive = false;
	gameOverTimerMs = 0;
  levelCompletedActive = false;
	levelCompletedTimerMs = 0;
   clockFreezeTimerMs = 0;
    playerShieldHitInvulnTimerMs = 0;
	enemyExplosions.clear();
	enemyBullets.clear();

	if(levelNum == 0)
	{
     currentLevelNum = 0;
        if(pendingItemRoomType == ITEM_ROOM_BOMB)
			init("levels/BombRoom.txt");
		else if(pendingItemRoomType == ITEM_ROOM_SHIELD)
			init("levels/ShieldRoom.txt");
       else if(pendingItemRoomType == ITEM_ROOM_EMPTY_KEY)
			init("levels/KeyRoom_collected.txt");
		else if(hasReturnPoint && containsDoorTile(collectedRoomKeys[returnLevelNum], returnTilePos))
		{
			init("levels/KeyRoom_collected.txt");
			cout << "Loading key room with collected keys: " << collectedRoomKeys[returnLevelNum].size() << endl;
		}
		else
			init("levels/KeyRoom.txt");

		for(int i = 0; i < carriedBombsBeforeRoom; ++i)
		{
			Bomb *carriedBomb = new Bomb();
			carriedBomb->init(glm::ivec2(0, 0), texProgram);
			carriedBomb->collect();
			bombs.push_back(carriedBomb);
		}
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
  freeShieldItems();
  freeClockItems();
	playerDeathActive = false;
    gameOverActive = false;
	gameOverTimerMs = 0;
  levelCompletedActive = false;
	levelCompletedTimerMs = 0;
   clockFreezeTimerMs = 0;
    playerShieldHitInvulnTimerMs = 0;
	enemyExplosions.clear();
	enemyBullets.clear();

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
   for(int i=0; i<int(suspendedShieldItems.size()); ++i) delete suspendedShieldItems[i];
   for(int i=0; i<int(suspendedClockItems.size()); ++i) delete suspendedClockItems[i];
	for(int i=0; i<int(suspendedEnemies.size()); ++i) delete suspendedEnemies[i];

	suspendedDoors.clear();
	suspendedKeys.clear();
	suspendedWeights.clear();
	suspendedBombs.clear();
   suspendedShieldItems.clear();
   suspendedClockItems.clear();
	suspendedEnemies.clear();
	suspendedWeightPushLatch.clear();
	suspendedEnemyBullets.clear();

	hasSuspendedLevel = false;
	suspendedLevelNum = -1;
   pendingCarryBombFromRoom = false;
	pendingCarryShieldFromRoom = false;
   carriedBombsBeforeRoom = 0;
}

void Scene::suspendCurrentLevelForKeyRoom()
{
	if(hasSuspendedLevel)
		return;

	hasSuspendedLevel = true;
	suspendedLevelNum = currentLevelNum;

	suspendedMap = map;           map = NULL;
	suspendedPlayer = player;     player = NULL;
	carriedBombsBeforeRoom = 0;
	for(int i = 0; i < int(bombs.size()); ++i)
	{
		if(bombs[i]->isCollected())
			++carriedBombsBeforeRoom;
	}

	suspendedDoors.swap(doors);
	suspendedKeys.swap(keys);
	suspendedWeights.swap(weights);
	suspendedBombs.swap(bombs);
 suspendedShieldItems.swap(shieldItems);
 suspendedClockItems.swap(clockItems);
	suspendedEnemies.swap(Enemies);
	suspendedWeightPushLatch.swap(weightPushLatch);
   suspendedEnemyBullets.swap(enemyBullets);
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
	freeShieldItems();
	freeClockItems();

	// Restore suspended level
	map = suspendedMap;               suspendedMap = NULL;
	player = suspendedPlayer;         suspendedPlayer = NULL;
	doors.swap(suspendedDoors);
	keys.swap(suspendedKeys);
	weights.swap(suspendedWeights);
	bombs.swap(suspendedBombs);
 shieldItems.swap(suspendedShieldItems);
 clockItems.swap(suspendedClockItems);
	Enemies.swap(suspendedEnemies);
	weightPushLatch.swap(suspendedWeightPushLatch);
	enemyBullets.swap(suspendedEnemyBullets);

	if(pendingCarryShieldFromRoom && player != NULL)
		player->activateShield();

	if(pendingCarryBombFromRoom)
	{
		Bomb *carriedBomb = new Bomb();
		carriedBomb->init(returnTilePos, texProgram);
		carriedBomb->collect();
		bombs.push_back(carriedBomb);
	}

	pendingCarryBombFromRoom = false;
	pendingCarryShieldFromRoom = false;
	pendingItemRoomType = ITEM_ROOM_KEY;
	carriedBombsBeforeRoom = 0;

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

int Scene::getKeyCount() const
{
	return totalKeys;
}

void Scene::addKeys(int number)
{
	totalKeys += number;
}

void Scene::setTotalKeys(int number)
{
	totalKeys = number;
}

void Scene::setNeededKeys()
{
	neededKeys = map->getKeyCount();
}