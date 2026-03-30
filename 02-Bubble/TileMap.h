#ifndef _TILE_MAP_INCLUDE
#define _TILE_MAP_INCLUDE


#include <glm/glm.hpp>
#include <set>
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

private:
	TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

public:
	// Tile maps can only be created inside an OpenGL context
	static TileMap *createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

	~TileMap();

	void render() const;
	void free();
	
	int getTileSize() const { return tileSize; }

	bool collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool collisionMoveDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const;
	bool isStairTile(const glm::ivec2& pos) const;
   bool isDoorTile(const glm::ivec2 &pos) const;
   bool isTubeTile(const glm::ivec2 &pos, bool topVariant) const;
	glm::ivec2 getTubeExit(const glm::ivec2 &entryTile) const;
  bool isTubeBottomTile(const glm::ivec2 &tilePos) const;
	glm::ivec2 worldToTile(const glm::vec2 &worldPos) const { return glm::ivec2(int(worldPos.x) / tileSize, int(worldPos.y) / tileSize); }
	glm::vec2 tileToWorld(const glm::ivec2 &tilePos) const { return glm::vec2(tilePos.x * tileSize, tilePos.y * tileSize); }
	glm::vec2 getMapSize() const;
	const vector<glm::ivec2> &getDoorPositions() const { return doorPositions; }
	
private:
	bool loadLevel(const string &levelFile);
	void prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program);

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
	int *map;
	set<int> collidedTiles;
  vector<glm::ivec2> doorPositions;
  vector<TubePair> tubeConnections;
  vector<glm::ivec2> tubeAlwaysBottomRenderTiles;
	int stair;
};


#endif // _TILE_MAP_INCLUDE


