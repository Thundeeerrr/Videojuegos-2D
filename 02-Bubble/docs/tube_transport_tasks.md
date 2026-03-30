# Tube Transport — Implementation Tasks

## Context
The player already has a door entry state machine (DoorState) as reference.  
Goal: two tube tiles (top/bottom) connected in pairs. Pressing ↑/↓ on a tube tile
triggers a 500ms travel animation, then the player rises out of the exit tube over 400ms.

---

## 1. `TileMap` — tube tile types and connections

- [x] Define two new sentinel values in the anonymous namespace of `TileMap.cpp`:  
      `TILE_TUBE_TOP = 998` (activated with ↓) and `TILE_TUBE_BOTTOM = 997` (activated with ↑)
- [x] Add a `TubePair` struct in `TileMap.h`:
      ```cpp
      struct TubePair { glm::ivec2 entry; glm::ivec2 exit; };
      ```
- [x] Add `vector<TubePair> tubeConnections;` as a private member in `TileMap.h`
- [x] In `loadLevel()`, parse tiles 997 and 998 the same way doors are parsed:  
      store them in two temporary vectors and pair them index-by-index into `tubeConnections`
- [x] In `loadLevel()`, set `map[j * mapSize.x + i] = -2` for tube tiles (distinct from door's `-1`)
- [x] Add `bool isTubeTile(const glm::ivec2 &pos, bool topVariant) const;` to `TileMap.h`
- [x] Implement `isTubeTile()` in `TileMap.cpp` probing bottom-centre of the player (same as `isStairTile`),  
      checking for `-2` and matching which variant (top/bottom) using `tubeConnections`
- [x] Add `glm::ivec2 getTubeExit(const glm::ivec2 &entryTile) const;` to `TileMap.h`
- [x] Implement `getTubeExit()`: iterate `tubeConnections`, match `entry` or `exit` field, return the other
- [x] Add `glm::ivec2 worldToTile(const glm::vec2 &worldPos) const;` to `TileMap.h`  
      implementation: `return glm::ivec2(int(worldPos.x) / tileSize, int(worldPos.y) / tileSize);`
- [x] Add `glm::vec2 tileToWorld(const glm::ivec2 &tilePos) const;` to `TileMap.h`  
      implementation: `return glm::vec2(tilePos.x * tileSize, tilePos.y * tileSize);`

---

## 2. `Player.h` — state machine and new members

- [x] Add or extend the tube state enum (separate from `DoorState`):
      ```cpp
      enum class TubeState { NONE, ENTERING, TRAVELING, EXITING, DONE };
      ```
- [x] Add private members:
      ```cpp
      TubeState tubeState;
      int tubeTimer;
      glm::ivec2 tubeExitPos;
      static const int TUBE_ENTER_TIME = 500;
      static const int TUBE_EXIT_TIME  = 400;
      ```
- [x] Add public methods:
      ```cpp
      bool isTubeEnterStarted() const;   // true when tubeState just became ENTERING
      bool isTubeTraveling() const;      // true when tubeState == TRAVELING
      bool isTubeDone() const;           // true when tubeState == DONE
      void setTubeExitPos(const glm::ivec2 &exitPos);
      void startTubeExit();              // sets tubeState = EXITING, resets tubeExitTimer
      void resetTubeState();
      ```

---

## 3. `Player.cpp` — init and update

- [x] In `Player::init()`, initialise: `tubeState = TubeState::NONE; tubeTimer = 0;`
- [x] In `Player::update()`, add early-return block **before** the door block:
  - If `ENTERING`: decrement `tubeTimer`, when ≤ 0 set `tubeState = TRAVELING`. Block all input. Return.
  - If `TRAVELING`: block all input, return (Scene handles the teleport this frame)
  - If `EXITING`: interpolate `posPlayer.y` from `tubeExitPos.y + 16` to `tubeExitPos.y` using  
    `t = 1.f - (tubeTimer / TUBE_EXIT_TIME)`, decrement timer, set `DONE` when ≤ 0. Return.
  - If `DONE`: block input, return (Scene resets this frame)
- [x] In `Player::update()`, after the stair/fall block, add tube activation:
      ```cpp
      bool onTubeTop    = map->isTubeTile(posPlayer, true);
      bool onTubeBottom = map->isTubeTile(posPlayer, false);
      if ((onTubeTop    && Game::instance().getKey(GLFW_KEY_DOWN)) ||
          (onTubeBottom && Game::instance().getKey(GLFW_KEY_UP)))
      {
          tubeState = TubeState::ENTERING;
          tubeTimer = TUBE_ENTER_TIME;
      }
      ```
- [x] Implement all new public methods declared in step 2
- [x] During `ENTERING` and `EXITING`, keep `sprite` in `STAND_LEFT` or `STAND_RIGHT`  
      (whichever matches `facingRight`) — do not change animation

---

## 4. `Scene` — orchestrate teleport and exit animation

- [x] In `Scene::update()`, after `player.update(deltaTime)`, check `player.isTubeTraveling()`:
  - Get player world position → convert to tile with `map->worldToTile(...)`
  - Get exit tile with `map->getTubeExit(playerTile)`
  - Convert exit tile to world with `map->tileToWorld(exitTile)`
  - Call `player.setTubeExitPos(exitTilePos)` and `player.startTubeExit()`
  - Immediately call `player.setPosition(exitWorld + glm::vec2(0, 16))` to place player  
    one tile below exit (enterrado) before the EXITING animation begins
- [x] In `Scene::update()`, check `player.isTubeDone()` → call `player.resetTubeState()`

---

## 5. Level file

- [x] In the level `.txt`, use `998` for the top tube tile and `997` for the bottom tube tile, the original tiles are 135 and 55 respectively in level03, so you can just replace those on this level
- [x] Ensure tubes are defined in matching pairs (first 998 connects to first 997, etc.)

---

## 6. Validation

- [ ] Walk player onto a `998` tile (top), press ↓ → 500ms freeze, then player appears  
      rising out of the `997` tile (bottom) over 400ms
- [ ] Same in reverse: `997` tile + ↑ → teleports to `998`
- [ ] Player cannot move or jump during the full ENTERING + EXITING sequence
- [ ] Tube does not activate while player is on a stair tile
- [ ] Tube does not activate while door state machine is active
