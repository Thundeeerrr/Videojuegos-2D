#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include "TileMap.h"
#include "Game.h"

using namespace std;

namespace
{
	const int TILE_DOOR = 999;
 const int TILE_TUBE_A_TOP = 992;
	const int TILE_TUBE_A_BOTTOM = 997;
	const int TILE_TUBE_B_TOP = 996;
	const int TILE_TUBE_B_BOTTOM = 995;
    const int TILE_TUBE_C_TOP = 994;
	const int TILE_TUBE_C_BOTTOM = 993;
 const int TILE_TUBE_D_TOP = 991;
	const int TILE_TUBE_D_BOTTOM = 990;
    const int DEFAULT_TUBE_TOP_RENDER_TILE_ID = 55;
	const int DEFAULT_TUBE_BOTTOM_RENDER_TILE_ID = 135;
	const int LEVEL04_TUBE_TOP_RENDER_TILE_ID = 36;
	const int LEVEL04_TUBE_BOTTOM_RENDER_TILE_ID = 113;
   const int LEVEL05_TUBE_TOP_RENDER_TILE_ID = 196;
	const int LEVEL05_TUBE_BOTTOM_RENDER_TILE_ID = 154;
}


TileMap *TileMap::createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	TileMap *map = new TileMap(levelFile, minCoords, program);
	
	return map;
}


TileMap::TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	loadLevel(levelFile);
	prepareArrays(minCoords, program);
}

TileMap::~TileMap()
{
	if(map != NULL)
		delete [] map;
	free();
}


void TileMap::render() const
{
	glEnable(GL_TEXTURE_2D);
	tilesheet.use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLES, 0, 6 * nTiles);
	glDisable(GL_TEXTURE_2D);
}

void TileMap::free()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

bool TileMap::loadLevel(const string &levelFile)
{
	ifstream fin;
	string line, tilesheetFile;
	stringstream sstream;
	char tile;
	
	fin.open(levelFile.c_str());
	if(!fin.is_open())
		return false;
	getline(fin, line);
	if(line.compare(0, 7, "TILEMAP") != 0)
		return false;
	getline(fin, line);
	sstream.str(line);
	sstream >> mapSize.x >> mapSize.y;
	getline(fin, line);
	sstream.str(line);
	sstream >> tileSize >> blockSize;
	getline(fin, line);
	sstream.str(line);
	sstream >> tilesheetFile;
   tubeTopRenderTileId = DEFAULT_TUBE_TOP_RENDER_TILE_ID;
	tubeBottomRenderTileId = DEFAULT_TUBE_BOTTOM_RENDER_TILE_ID;
	if(tilesheetFile.find("level4-def") != string::npos)
	{
		tubeTopRenderTileId = LEVEL04_TUBE_TOP_RENDER_TILE_ID;
		tubeBottomRenderTileId = LEVEL04_TUBE_BOTTOM_RENDER_TILE_ID;
	}
   else if(tilesheetFile.find("level5-def") != string::npos)
	{
		tubeTopRenderTileId = LEVEL05_TUBE_TOP_RENDER_TILE_ID;
		tubeBottomRenderTileId = LEVEL05_TUBE_BOTTOM_RENDER_TILE_ID;
	}
	tilesheet.loadFromFile(tilesheetFile, TEXTURE_PIXEL_FORMAT_RGBA);
	tilesheet.setWrapS(GL_CLAMP_TO_EDGE);
	tilesheet.setWrapT(GL_CLAMP_TO_EDGE);
	tilesheet.setMinFilter(GL_NEAREST);
	tilesheet.setMagFilter(GL_NEAREST);
	getline(fin, line);
	sstream.str(line);
	sstream >> tilesheetSize.x >> tilesheetSize.y;
	tileTexSize = glm::vec2(1.f / tilesheetSize.x, 1.f / tilesheetSize.y);

	// Read the collided tiles, first read the quantity of collided tiles and then the tile ids. Clear the set of collided tiles before reading them in case of changing levels.
	collidedTiles.clear();
 doorPositions.clear();
	tubeConnections.clear();
   tubeAlwaysBottomRenderTiles.clear();
    vector<glm::ivec2> tubeATopTiles;
	vector<glm::ivec2> tubeABottomTiles;
	vector<glm::ivec2> tubeBTopTiles;
	vector<glm::ivec2> tubeBBottomTiles;
 vector<glm::ivec2> tubeCTopTiles;
	vector<glm::ivec2> tubeCBottomTiles;
 vector<glm::ivec2> tubeDTopTiles;
	vector<glm::ivec2> tubeDBottomTiles;
	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	int numCollidedTiles, cTileId;
	if (sstream >> numCollidedTiles) 
	{
		for (int i = 0; i < numCollidedTiles; ++i) 
		{
			sstream >> cTileId;
			collidedTiles.insert(cTileId);
		}
	}

	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	sstream >> stair;

	map = new int[mapSize.x * mapSize.y];
	int tileId;
	for (int j = 0; j < mapSize.y; j++)
	{
		for (int i = 0; i < mapSize.x; i++)
		{
			fin >> tileId; // This safely reads 1, 9, 10, or 45, automatically ignoring spaces!
            if(tileId == TILE_DOOR)
			{
				doorPositions.push_back(glm::ivec2(i, j));
				map[j * mapSize.x + i] = -1;
			}
            else if(tileId == TILE_TUBE_A_TOP)
			{
               tubeATopTiles.push_back(glm::ivec2(i, j));
				map[j * mapSize.x + i] = -2;
			}
         else if(tileId == TILE_TUBE_A_BOTTOM)
			{
                tubeABottomTiles.push_back(glm::ivec2(i, j));
				map[j * mapSize.x + i] = -2;
			}
			else if(tileId == TILE_TUBE_B_TOP)
			{
				tubeBTopTiles.push_back(glm::ivec2(i, j));
				map[j * mapSize.x + i] = -2;
			}
			else if(tileId == TILE_TUBE_B_BOTTOM)
			{
				tubeBBottomTiles.push_back(glm::ivec2(i, j));
				map[j * mapSize.x + i] = -2;
			}
            else if(tileId == TILE_TUBE_C_TOP)
			{
				tubeCTopTiles.push_back(glm::ivec2(i, j));
				map[j * mapSize.x + i] = -2;
			}
			else if(tileId == TILE_TUBE_C_BOTTOM)
			{
				tubeCBottomTiles.push_back(glm::ivec2(i, j));
				map[j * mapSize.x + i] = -2;
			}
            else if(tileId == TILE_TUBE_D_TOP)
			{
				tubeDTopTiles.push_back(glm::ivec2(i, j));
				tubeAlwaysBottomRenderTiles.push_back(glm::ivec2(i, j));
				map[j * mapSize.x + i] = -2;
			}
			else if(tileId == TILE_TUBE_D_BOTTOM)
			{
				tubeDBottomTiles.push_back(glm::ivec2(i, j));
				tubeAlwaysBottomRenderTiles.push_back(glm::ivec2(i, j));
				map[j * mapSize.x + i] = -2;
			}
			else
				map[j * mapSize.x + i] = tileId;
		}
	}

   int pairedTubeCountA = min(int(tubeATopTiles.size()), int(tubeABottomTiles.size()));
	for(int i = 0; i < pairedTubeCountA; ++i)
	{
		TubePair pair;
       pair.entry = tubeATopTiles[i];
		pair.exit = tubeABottomTiles[i];
		tubeConnections.push_back(pair);
	}

	int pairedTubeCountB = min(int(tubeBTopTiles.size()), int(tubeBBottomTiles.size()));
	for(int i = 0; i < pairedTubeCountB; ++i)
	{
		TubePair pair;
		pair.entry = tubeBTopTiles[i];
		pair.exit = tubeBBottomTiles[i];
		tubeConnections.push_back(pair);
	}

	int pairedTubeCountC = min(int(tubeCTopTiles.size()), int(tubeCBottomTiles.size()));
	for(int i = 0; i < pairedTubeCountC; ++i)
	{
		TubePair pair;
		pair.entry = tubeCTopTiles[i];
		pair.exit = tubeCBottomTiles[i];
		tubeConnections.push_back(pair);
	}

	int pairedTubeCountD = min(int(tubeDTopTiles.size()), int(tubeDBottomTiles.size()));
	for(int i = 0; i < pairedTubeCountD; ++i)
	{
		TubePair pair;
		pair.entry = tubeDTopTiles[i];
		pair.exit = tubeDBottomTiles[i];
		tubeConnections.push_back(pair);
	}
	fin.close();
	
	return true;
}

void TileMap::prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program)
{
	int tile;
	glm::vec2 posTile, texCoordTile[2], halfTexel;
	vector<float> vertices;
	
	nTiles = 0;
	halfTexel = glm::vec2(0.5f / tilesheet.width(), 0.5f / tilesheet.height());
	for(int j=0; j<mapSize.y; j++)
	{
		for(int i=0; i<mapSize.x; i++)
		{
			tile = map[j * mapSize.x + i];
         int tileToRender = tile;
			if(tile == -2)
			{
				glm::ivec2 tilePos(i, j);
                bool forcedBottomRender = false;
				for(int r = 0; r < int(tubeAlwaysBottomRenderTiles.size()); ++r)
				{
					if(tubeAlwaysBottomRenderTiles[r] == tilePos)
					{
						forcedBottomRender = true;
						break;
					}
				}
				if(forcedBottomRender)
					tileToRender = tubeBottomRenderTileId;
				else
				for(int k = 0; k < int(tubeConnections.size()); ++k)
				{
					if(tubeConnections[k].entry == tilePos)
					{
                        tileToRender = tubeTopRenderTileId;
						break;
					}
					if(tubeConnections[k].exit == tilePos)
					{
                     tileToRender = tubeBottomRenderTileId;
						break;
					}
				}
			}
		   if(tileToRender < 0)
				continue;
			// Non-empty tile
			nTiles++;
			posTile = glm::vec2(minCoords.x + i * tileSize, minCoords.y + j * tileSize);
            texCoordTile[0] = glm::vec2(float((tileToRender)%tilesheetSize.x) / tilesheetSize.x, float((tileToRender)/tilesheetSize.x) / tilesheetSize.y);
			texCoordTile[1] = texCoordTile[0] + tileTexSize;
			//texCoordTile[0] += halfTexel;
			texCoordTile[1] -= halfTexel;
			// First triangle
			vertices.push_back(posTile.x); vertices.push_back(posTile.y);
			vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
			vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y);
			vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[0].y);
			vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
			vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
			// Second triangle
			vertices.push_back(posTile.x); vertices.push_back(posTile.y);
			vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
			vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
			vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
			vertices.push_back(posTile.x); vertices.push_back(posTile.y + blockSize);
			vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[1].y);
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * nTiles * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	posLocation = program.bindVertexAttribute("position", 2, 4*sizeof(float), 0);
	texCoordLocation = program.bindVertexAttribute("texCoord", 2, 4*sizeof(float), (void *)(2*sizeof(float)));
}

// Collision tests for axis aligned bounding boxes.
// Method collisionMoveDown also corrects Y coordinate if the box is
// already intersecting a tile below.

bool TileMap::collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	/*int x, y0, y1;
	
	x = pos.x / tileSize;
	y0 = pos.y / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;
	for(int y=y0; y<=y1; y++)
	{
		if(map[y*mapSize.x+x] != 0)
			return true;
	}
	
	return false;*/
	if (pos.x <= 0)
		return true;
	return false;
}

bool TileMap::collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	/*int x, y0, y1;
	
	x = (pos.x + size.x - 1) / tileSize;
	y0 = pos.y / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;
	for(int y=y0; y<=y1; y++)
	{
		if(map[y*mapSize.x+x] != 0)
			return true;
	}
	
	return false;*/
	if (pos.x + size.x > mapSize.x * tileSize)
		return true;

	return false;
}

bool TileMap::collisionMoveDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const
{
	int x0, x1, y;
	
	x0 = pos.x / tileSize;
	x1 = (pos.x + size.x - 1) / tileSize;
	y = (pos.y + size.y - 1) / tileSize;
	bool tryToClimbDown = Game::instance().getKey(GLFW_KEY_DOWN);
	for(int x=x0; x<=x1; x++)
	{
		int tile = map[y * mapSize.x + x];
		if (tryToClimbDown) cout << "Trying to climb down. Checking tile at (" << x << ", " << y << ") with tile ID: " << tile << endl;
     if(collidedTiles.find(tile) != collidedTiles.end() || tile == -2)
		{
			if (*posY - tileSize * y + size.y <= 4)
			{
				*posY = tileSize * y - size.y;
				return true;
			}
			//return true;
		}
	}
	
	return false;
}

bool TileMap::isStairTile(const glm::ivec2& pos) const
{
	// Probe point = bottom-center of a 16x16 player.
	// If you later change player size, refactor these into parameters.
	const int playerW = 16;
	const int playerH = 16;

	const int probeX = pos.x + playerW / 2;
	const int probeY0 = pos.y + playerH;
	const int probeY1 = pos.y + playerH - 1;

	const int x = probeX / tileSize;
	const int y0 = probeY0 / tileSize;
	const int y1 = probeY1 / tileSize;


	return map[y0 * mapSize.x + x] == stair || map[y1 * mapSize.x + x] == stair;
}

bool TileMap::isDoorTile(const glm::ivec2 &pos) const
{
	const int playerW = 16;
	const int playerH = 16;

	const int probeX = pos.x + playerW / 2;
	const int probeY = pos.y + playerH - 1;

	const int x = probeX / tileSize;
	const int y = probeY / tileSize;

	if(x < 0 || x >= mapSize.x || y < 0 || y >= mapSize.y)
		return false;

	return map[y * mapSize.x + x] == -1;
}

bool TileMap::isTubeTile(const glm::ivec2 &pos, bool topVariant) const
{
	const int playerW = 16;
	const int playerH = 16;
	const int tubeProbeExtraYPx = 8;

	const int probeX = pos.x + playerW / 2;
 const int probeY0 = pos.y + playerH;
	const int probeY1 = pos.y + playerH - 1;
	const int probeY2 = pos.y + playerH + tubeProbeExtraYPx;
	const int probeY3 = pos.y + playerH + tubeProbeExtraYPx - 1;

	const int x = probeX / tileSize;
    const int y0 = probeY0 / tileSize;
	const int y1 = probeY1 / tileSize;
	const int y2 = probeY2 / tileSize;
	const int y3 = probeY3 / tileSize;

  if(x < 0 || x >= mapSize.x)
		return false;

    auto isDesiredTubeVariant = [&](const glm::ivec2 &tilePos) -> bool
	{
        for(int i = 0; i < int(tubeAlwaysBottomRenderTiles.size()); ++i)
		{
			if(tubeAlwaysBottomRenderTiles[i] == tilePos)
				return !topVariant;
		}

		for(int i = 0; i < int(tubeConnections.size()); ++i)
		{
			if(topVariant && tubeConnections[i].entry == tilePos)
				return true;
			if(!topVariant && tubeConnections[i].exit == tilePos)
				return true;
		}
		return false;
	};

	if(y0 >= 0 && y0 < mapSize.y)
	{
		if(map[y0 * mapSize.x + x] == -2 && isDesiredTubeVariant(glm::ivec2(x, y0)))
			return true;
	}

	if(y1 >= 0 && y1 < mapSize.y)
	{
		if(map[y1 * mapSize.x + x] == -2 && isDesiredTubeVariant(glm::ivec2(x, y1)))
			return true;
	}

	if(y2 >= 0 && y2 < mapSize.y)
	{
		if(map[y2 * mapSize.x + x] == -2 && isDesiredTubeVariant(glm::ivec2(x, y2)))
			return true;
	}

	if(y3 >= 0 && y3 < mapSize.y)
	{
		if(map[y3 * mapSize.x + x] == -2 && isDesiredTubeVariant(glm::ivec2(x, y3)))
			return true;
	}

	return false;
}

glm::ivec2 TileMap::getTubeExit(const glm::ivec2 &entryTile) const
{
	for(int i = 0; i < int(tubeConnections.size()); ++i)
	{
		if(tubeConnections[i].entry == entryTile)
			return tubeConnections[i].exit;
		if(tubeConnections[i].exit == entryTile)
			return tubeConnections[i].entry;
	}

	return entryTile;
}

bool TileMap::isTubeBottomTile(const glm::ivec2 &tilePos) const
{
    for(int i = 0; i < int(tubeAlwaysBottomRenderTiles.size()); ++i)
	{
		if(tubeAlwaysBottomRenderTiles[i] == tilePos)
			return true;
	}

	for(int i = 0; i < int(tubeConnections.size()); ++i)
	{
		if(tubeConnections[i].exit == tilePos)
			return true;
		if(tubeConnections[i].entry == tilePos)
			return false;
	}

	return false;
}

glm::vec2 TileMap::getMapSize() const
{
	return mapSize;
}




























