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
#include "Pushable.h"
#include "Bomb.h"
#include "ShieldItem.h"
#include "ClockItem.h"
#include "Sprite.h"

// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{
	struct EnemyExplosion
	{
		glm::vec2 pos;
		int timerMs;
	};

	struct EnemyBullet
	{
		glm::vec2 pos;
		int dirSign;
	};

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
	void resetForNewGame();
	void restartCurrentLevel();

private:
	void initShaders();
	void freeDoors();
    void freeKeys();
	void freeEnemies();
    void freeBombs();
    void freeShieldItems();
    void freeClockItems();
	int findClosestDoorIndex(const glm::ivec2 &playerTilePos) const;

	void suspendCurrentLevelForKeyRoom();
	void restoreSuspendedLevelFromKeyRoom();
	void clearSuspendedLevel();

private:
	TileMap *map;
	Player *player;
   std::vector<Door*> doors;
   std::vector<Key*> keys;
    std::vector<Pushable*> weights;
   std::vector<Bomb*> bombs;
   std::vector<ShieldItem*> shieldItems;
   std::vector<ClockItem*> clockItems;
	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;
	bool dWasPressed;
	bool hasDoorTarget;
   bool doorTargetIsLockedExit;
	glm::ivec2 doorTargetTilePos;
	bool hasSpawnOverride;
	glm::ivec2 spawnTileOverride;
  int pendingItemRoomType;
  bool pendingCarryBombFromRoom;
	bool pendingCarryShieldFromRoom;
  int carriedBombsBeforeRoom;
    bool spawnAtDoorInLoadedLevel;
	int currentLevelNum;
	bool hasReturnPoint;
	int returnLevelNum;
	glm::ivec2 returnTilePos;
	std::map<int, std::vector<glm::ivec2> > openedDoorsByLevel;
	std::map<int, std::vector<glm::ivec2> > collectedRoomKeys;
	vector<Enemy*> Enemies;
	bool godMode;
 std::vector<bool> weightPushLatch;
	bool spaceWasPressed;
	Texture explosionTexture;
	GLuint explosionVao, explosionVbo;
	GLint explosionPosLocation, explosionTexCoordLocation;
    Texture enemyBulletTexture;
	GLuint enemyBulletVao, enemyBulletVbo;
	GLint enemyBulletPosLocation, enemyBulletTexCoordLocation;
    Texture gameOverTexture;
    Texture levelCompletedTexture;
    Texture nextLevelTexture;
	GLuint gameOverVao, gameOverVbo;
	GLint gameOverPosLocation, gameOverTexCoordLocation;
    Texture pauseMenuTexture;
	Texture bombHudTexture;
 Texture hudKeyTexture;
	Texture keyCounterTexture;
  Texture starsTexture;
	GLuint bombHudVao, bombHudVbo;
	GLint bombHudPosLocation, bombHudTexCoordLocation;
  GLuint keyHudVao, keyHudVbo;
	GLint keyHudPosLocation, keyHudTexCoordLocation;
  GLuint keyCounterVao, keyCounterVbo;
	GLint keyCounterPosLocation, keyCounterTexCoordLocation;
 std::vector<Sprite*> levelCompletedStars;
	bool playerDeathActive;
  bool gameOverActive;
	int gameOverTimerMs;
  bool levelCompletedActive;
	int levelCompletedTimerMs;
 int playerShieldHitInvulnTimerMs;
 int clockFreezeTimerMs;
  bool pauseActive;
	bool pWasPressed;
	bool hWasPressed;
	std::vector<EnemyExplosion> enemyExplosions;
 std::vector<EnemyBullet> enemyBullets;
	int remainingLives;
	static const int MAX_LIVES = 3;

	// Suspended level state (used only for key-room round trips).
	bool hasSuspendedLevel;
	int suspendedLevelNum;
	TileMap *suspendedMap;
	Player *suspendedPlayer;
	std::vector<Door*> suspendedDoors;
	std::vector<Key*> suspendedKeys;
	std::vector<Pushable*> suspendedWeights;
	std::vector<Bomb*> suspendedBombs;
   std::vector<ShieldItem*> suspendedShieldItems;
   std::vector<ClockItem*> suspendedClockItems;
	std::vector<Enemy*> suspendedEnemies;
	std::vector<bool> suspendedWeightPushLatch;
  std::vector<EnemyBullet> suspendedEnemyBullets;
};


#endif // _SCENE_INCLUDE

