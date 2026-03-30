#ifndef _TILE_MAP_INCLUDE
#define _TILE_MAP_INCLUDE


#include <glm/glm.hpp>
#include <set>
#include <unordered_set>
#include <vector>
#include "Texture.h"
#include "ShaderProgram.h"


// Class Tilemap is capable of loading a tile map from a text file in a very
// simple format (see level01.txt for an example). With this information
// it builds a single VBO that contains all tiles. As a result the render
// method draws the whole map independently of what is visible.


class TileMap
{

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


private:
	GLuint vao;
	GLuint vbo;
	GLint posLocation, texCoordLocation;
	int nTiles;
	glm::ivec2 position, mapSize, tilesheetSize;
	int tileSize, blockSize;
	Texture tilesheet;
	glm::vec2 tileTexSize;
	int* map;
	set<int> collidedTiles;
	vector<glm::ivec2> doorPositions;
	int stair;
	std::unordered_set<glm::ivec2, IVec2Hash> keyPositions;


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
	bool isKeyTile(const glm::ivec2& pos) const;
	glm::vec2 getMapSize() const;
	const vector<glm::ivec2> &getDoorPositions() const { return doorPositions; }
	const std::unordered_set<glm::ivec2, IVec2Hash>& getKeyPositions() const { return keyPositions; }
	void removeKeyAtTile(const glm::ivec2& tilePos);

private:
	bool loadLevel(const string &levelFile);
	void prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program);


};


#endif // _TILE_MAP_INCLUDE


