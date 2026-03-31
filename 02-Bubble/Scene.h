#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include <vector>
#include <map>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Door.h"
#include "Key.h"
#include "Enemy.h"

// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();

	void init(const std::string& sceneName);
	void update(int deltaTime);
	void render();
	void toggleGodMode();
	void giveAllKeys();
	void loadLevel(int levelNum);
 void loadLevelFile(const std::string &levelPath);
	bool isGodMode() const;

private:
	void initShaders();
	void freeDoors();
    void freeKeys();
	void freeEnemies();
	int findClosestDoorIndex(const glm::ivec2 &playerTilePos) const;

private:
	TileMap *map;
	Player *player;
   std::vector<Door*> doors;
   std::vector<Key*> keys;
	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;
	bool dWasPressed;
	bool hasDoorTarget;
	glm::ivec2 doorTargetTilePos;
	bool hasSpawnOverride;
	glm::ivec2 spawnTileOverride;
    bool spawnAtDoorInLoadedLevel;
	int currentLevelNum;
	bool hasReturnPoint;
	int returnLevelNum;
	glm::ivec2 returnTilePos;
	std::map<int, std::vector<glm::ivec2> > openedDoorsByLevel;
	vector<Enemy*> Enemies;
	bool godMode;
};


#endif // _SCENE_INCLUDE

