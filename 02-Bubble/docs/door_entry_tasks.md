# Door Entry Behaviour — Implementation Tasks

## Context
The door toggle animation already works (triggered with `D` key).  
Goal: trigger it with ↑ arrow when the player stands on a door tile, freeze the player ~1 s, then transition to the Key Room state.

---

## 1. `TileMap` — expose door tile detection

- [ ] Add declaration `bool isDoorTile(const glm::ivec2 &pos) const;` to `TileMap.h`
- [ ] Implement `isDoorTile()` in `TileMap.cpp`, probing `map[probeY * mapSize.x + probeX] == -1`  
      (probe point: bottom-centre of the 16×16 player, same as `isStairTile`)

---

## 2. `Door` — explicit open method

- [ ] Add declaration `void open();` and `bool getIsOpen() const { return isOpen; }` to `Door.h`
- [ ] Implement `Door::open()` in `Door.cpp`: calls `toggleOpen()` only if `!isOpen`
- [ ] Remove (or keep for debug) the `D`-key call that currently triggers `toggleOpen()`

---

## 3. `Player` — door state machine

- [ ] Add private enum in `Player.h`:  
      `enum class DoorState { NONE, ENTERING, ENTERED };`
- [ ] Add private members in `Player.h`: `DoorState doorState;` and `int doorTimer;`
- [ ] Add public methods in `Player.h`:  
      `bool isDoorInteractionStarted() const;`  
      `bool hasDoorTransitionEnded() const;`  
      `void resetDoorState();`
- [ ] Initialise `doorState = DoorState::NONE;` and `doorTimer = 0;` in `Player::init()`
- [ ] In `Player::update()`, add early-return block at the top:  
      if `ENTERING` → decrement timer, set `ENTERED` when ≤ 0, skip all movement
- [ ] In `Player::update()`, after the stair/fall block, add door detection:  
      if `!isTouchingStair && KEY_UP pressed && map->isDoorTile(posPlayer)` → set `ENTERING` + timer
- [ ] Implement the three new public methods (`isDoorInteractionStarted`, `hasDoorTransitionEnded`, `resetDoorState`)

---

## 4. `Scene` — orchestrate door + transition

- [ ] In `Scene::update()`, after `player.update(deltaTime)`, check `player.isDoorInteractionStarted()`  
      → find the nearest `Door` and call `door.open()`
- [ ] In `Scene::update()`, check `player.hasDoorTransitionEnded()`  
      → call `player.resetDoorState()` and trigger the Key Room state change
- [ ] Verify door lookup logic (match player tile position against `door.getTilePos()` or world position)

---

## 5. Validation

- [ ] Walk Bugs to a door tile, press ↑ → door opens immediately
- [ ] Player is frozen for ~1 s while the door open animation plays
- [ ] After the timer, the game transitions to the Key Room state
- [ ] Player **cannot** enter a door while on a stair tile (priority check)
- [ ] `D` debug key removed or guarded behind a `#ifdef DEBUG` flag
