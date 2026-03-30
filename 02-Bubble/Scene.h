#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include <vector>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Door.h"


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
	int findClosestDoorIndex(const glm::ivec2 &playerTilePos) const;

private:
	TileMap *map;
	Player *player;
   std::vector<Door*> doors;
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

	bool godMode;
};


#endif // _SCENE_INCLUDE

