#include <iostream>
#include <cmath>
#include <sstream>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"


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
}


void Scene::init(const std::string &sceneName)
{
	initShaders();
	freeDoors();
	freeKeys();
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
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
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
	bool dPressed = Game::instance().getKey(GLFW_KEY_D);
	if(dPressed && !dWasPressed)
	{
		for(int i=0; i<int(doors.size()); ++i)
			doors[i]->toggleOpen();
	}
	dWasPressed = dPressed;

	for(int i=0; i<int(doors.size()); ++i)
		doors[i]->update(deltaTime);
	player->update(deltaTime);

	glm::vec2 playerPos = player->getPosition();
	glm::ivec2 playerTilePos((int(playerPos.x) + 8) / map->getTileSize(),(int(playerPos.y) + 15) / map->getTileSize());

	for (int i = 0; i < int(keys.size()); )
	{
		if (keys[i]->getTilePos() == playerTilePos)
		{
			cout << "Key collected at tile (" << playerTilePos.x << ", " << playerTilePos.y << ")" << endl;
			player->addKey();
			map->removeKeyAtTile(keys[i]->getTilePos());
			delete keys[i];
			keys.erase(keys.begin() + i);
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
            if(hasReturnPoint)
			{
				hasSpawnOverride = true;
				spawnTileOverride = returnTilePos;
				loadLevel(returnLevelNum);
				return;
			}
		}
		else
		{
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
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	map->render();
   for(int i=0; i<int(doors.size()); ++i)
		doors[i]->render();
   for(int i=0; i<int(keys.size()); ++i)
		keys[i]->render();
	player->render();
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

void Scene::loadLevel(int levelNum)
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

	if(levelNum == 0)
	{
     currentLevelNum = 0;
		init("levels/KeyRoom.txt");
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

