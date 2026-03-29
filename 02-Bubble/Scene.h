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
	void loadLevelFile(const std::string &levelPath); // Added back for clarity

private:
	void initShaders();
	void freeDoors();

private:
	TileMap *map;
	Player *player;
   std::vector<Door*> doors;
	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;
	bool dWasPressed;

};


#endif // _SCENE_INCLUDE

