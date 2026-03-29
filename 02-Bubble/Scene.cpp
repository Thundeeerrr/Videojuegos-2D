#include <iostream>
#include <cmath>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"


#define SCREEN_X 0
#define SCREEN_Y 0

#define INIT_PLAYER_X_TILES 9
#define INIT_PLAYER_Y_TILES 4


Scene::Scene()
{
	map = NULL;
	player = NULL;
   dWasPressed = false;
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
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);
	projection = glm::ortho(0.f, float(map->getMapSize().x * map->getTileSize()), float(map->getMapSize().y * map->getTileSize()), 0.f);
	//projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
	currentTime = 0.0f;
   dWasPressed = false;
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
		init("levels/KeyRoom.txt");
		return;
	}
	if(levelNum < 1)
		return;

	stringstream levelPath;
	levelPath << "levels/level";
	if(levelNum < 10)
		levelPath << '0';
	levelPath << levelNum << ".txt";

	init(levelPath.str());
}

bool Scene::isGodMode() const
{
	return godMode;
}

