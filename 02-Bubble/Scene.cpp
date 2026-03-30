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
}


void Scene::init(const std::string &sceneName)
{
	initShaders();
   freeDoors();
	map = TileMap::createTileMap(sceneName, glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
 const std::vector<glm::ivec2> &doorPositions = map->getDoorPositions();
	for(int i=0; i<int(doorPositions.size()); ++i)
	{
		Door *door = new Door();
		door->init(doorPositions[i], texProgram);
		doors.push_back(door);
	}
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
    bool shouldPlayDoorExitAnimation = false;
    bool shouldKeepSpawnDoorOpen = false;
 glm::ivec2 spawnTile(INIT_PLAYER_X_TILES, INIT_PLAYER_Y_TILES);
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
	player->render();
}

void Scene::freeDoors()
{
	for(int i=0; i<int(doors.size()); ++i)
		delete doors[i];
	doors.clear();
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

