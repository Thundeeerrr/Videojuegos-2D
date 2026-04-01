#ifndef _TILE_MAP_INCLUDE
#define _TILE_MAP_INCLUDE


#include <glm/glm.hpp>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "Texture.h"
#include "ShaderProgram.h"


// Class Tilemap is capable of loading a tile map from a text file in a very
// simple format (see level01.txt for an example). With this information
// it builds a single VBO that contains all tiles. As a result the render
// method draws the whole map independently of what is visible.


class TileMap
{
	struct TubePair { glm::ivec2 entry; glm::ivec2 exit; };

public:
	struct IVec2Hash {
		size_t operator()(const glm::ivec2& v) const noexcept {
			size_t h1 = std::hash<int>{}(v.x);
			size_t h2 = std::hash<int>{}(v.y);
			return h1 ^ (h2 << 1);
		}
	};

private:
	TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

public:
   static const int WARP_TILE_FLOOR = 989;
	static const int WARP_TILE_NO_FLOOR = 988;
	static const int JUMP_PLATFORM_TILE = 987;
	static const int DOOR_TILE_STAIRS = 986;
	static const int DOOR_TILE_NO_STAIRS = 985;
	static const int WEIGHT_TILE = 984;
 static const int CLOCK_TILE = 981;
	static const int SHIELD_TILE = 982;

	// Tile maps can only be created inside an OpenGL context
	static TileMap *createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

	~TileMap();

	void render() const;
	void free();
	
	int getTileSize() const { return tileSize; }

	bool collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool collisionMoveDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const;
  bool collisionMoveUp(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const;
	bool isStairTileForBody(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool isStairTile(const glm::ivec2& pos) const; // keep for Player compatibility
    bool isDoorTile(const glm::ivec2 &tilePos) const;
   bool isLockedExitDoorObject(const glm::ivec2 &tilePos) const;
	bool hasLockedExitDoorObject() const;
   bool isTubeTile(const glm::ivec2 &pos, bool topVariant) const;
	glm::ivec2 getTubeExit(const glm::ivec2 &entryTile) const;
  bool isTubeBottomTile(const glm::ivec2 &tilePos) const;
	glm::ivec2 worldToTile(const glm::vec2 &worldPos) const { return glm::ivec2(int(worldPos.x) / tileSize, int(worldPos.y) / tileSize); }
	glm::vec2 tileToWorld(const glm::ivec2 &tilePos) const { return glm::vec2(tilePos.x * tileSize, tilePos.y * tileSize); }
	bool isKeyTile(const glm::ivec2& pos) const;
	glm::vec2 getMapSize() const;
	const vector<glm::ivec2> &getDoorPositions() const { return doorPositions; }
	const std::unordered_set<glm::ivec2, IVec2Hash>& getKeyPositions() const { return keyPositions; }
    const vector<glm::ivec2> &getWeightPositions() const { return weightPositions; }
    const vector<glm::ivec2> &getClockPositions() const { return clockPositions; }
    const vector<glm::ivec2> &getShieldPositions() const { return shieldPositions; }
    const vector<glm::ivec2> &getBombPositions() const { return bombPositions; }
	void removeKeyAtTile(const glm::ivec2& tilePos);
	int getTile(int x, int y) const;
	std::vector<std::pair<glm::ivec2, glm::ivec2>> getWarpPlatformPairs() const;
	void addDoorLink(glm::ivec2 a, glm::ivec2 b);
	void clearDoorLinks();
	glm::ivec2 getDoorDestination(glm::ivec2 tilePos) const;

private:
	bool loadLevel(const string &levelFile);
	void prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program);
	int encodeTile(glm::ivec2 pos) const;
	glm::ivec2 decodeTile(int key) const;

private:
	GLuint vao;
	GLuint vbo;
	GLint posLocation, texCoordLocation;
	int nTiles;
	glm::ivec2 position, mapSize, tilesheetSize;
	int tileSize, blockSize;
	Texture tilesheet;
	glm::vec2 tileTexSize;
   int tubeTopRenderTileId;
	int tubeBottomRenderTileId;
   int jumpPlatformRenderTileId;
   int doorTileStairsRenderTileId;
	int doorTileNoStairsRenderTileId;
	int *map;
	set<int> collidedTiles;
	vector<glm::ivec2> doorPositions;
  std::unordered_set<glm::ivec2, IVec2Hash> lockedExitDoorPositions;
   vector<glm::ivec2> weightPositions;
 vector<glm::ivec2> clockPositions;
    vector<glm::ivec2> shieldPositions;
   vector<glm::ivec2> bombPositions;
	vector<TubePair> tubeConnections;
	vector<glm::ivec2> tubeAlwaysBottomRenderTiles;
	int stair;
	std::unordered_set<glm::ivec2, IVec2Hash> keyPositions;
  std::unordered_map<int, int> doorLinks;
  int neededKeys;
  int totalKeys;
};


#endif // _TILE_MAP_INCLUDE


