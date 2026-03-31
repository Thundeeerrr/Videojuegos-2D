# Task: Implement Main Menu & State Machine for Bugs Bunny Crazy Castle 3 Clone

## Context

This is a 2D game project built in **C++ with OpenGL (modern, shader-based)** and **GLM**.
The rendering pipeline currently uses a `ShaderProgram` (in `Scene`) with `projection` + `modelview` + `texCoordDispl` uniforms,
a `Texture` class wrapping OpenGL textures, a `Sprite` class handling VAO/VBO + keyframe animations,
and a `TileMap` class that reads `.txt` level files and renders via a single `glDrawArrays` call.

`Game` is a **singleton**. Currently it delegates everything directly to `Scene`.
The goal of this task is to introduce a **state machine** and a **Menu screen** as the first state.

---

## Existing Files (DO NOT modify unless instructed)

| File | Role |
|---|---|
| `main.cpp` | GLFW window + game loop. Calls `Game::instance().init/update/render/keyPressed/keyReleased/mouseMove/mousePress/mouseRelease` |
| `Shader.cpp/.h` | Compiles GLSL shaders from file |
| `ShaderProgram.cpp/.h` | Links shaders, exposes `setUniformMatrix4f`, `setUniform4f`, `setUniform2f`, `bindVertexAttribute` |
| `Texture.cpp/.h` | Loads PNG via `loadFromFile(path, TEXTURE_PIXEL_FORMAT_RGBA)`, wraps GL texture |
| `Sprite.cpp/.h` | Creates a quad VAO/VBO, supports keyframe animations via `addKeyframe`, `changeAnimation`, `update`, `render` |
| `TileMap.cpp/.h` | Reads level `.txt`, builds tile mesh, exposes `collisionMoveLeft/Right/Down` |
| `Scene.cpp/.h` | Owns `TileMap* map` and `Player* player`. Has `initShaders()`, `init()`, `update(dt)`, `render()` |
| `Player.cpp/.h` | Reads GLFW keys via `Game::instance().getKey(key)`. Has jump physics (to be replaced later) |

### Key constants (defined in `Game.h`)
```cpp
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
```

### Shader uniforms expected by `shaders/texture.vert` / `shaders/texture.frag`
```glsl
uniform mat4 projection;
uniform mat4 modelview;
uniform vec4 color;
uniform vec2 texCoordDispl;
```

---

## Spritesheet Asset

A **single spritesheet** is used for all menu visuals. Do NOT use separate cropped images.

**File:** `images/MainMenu.png`

This is the original title screen spritesheet (779×294 px) containing all start screen variants
arranged in a 4-column × 2-row grid. Each cell is approximately **194×147 px**.

| Column | Row | Content |
|--------|-----|---------|
| 0 | 0 | KEMCO logo screen |
| 1 | 0 | Copyright text screen |
| 2 | 0 | **Title screen — cyan background** ✅ use this |
| 3 | 0 | Title screen — pink background |
| 0–3 | 1 | Greyscale variants (not used) |

### Precomputed UV values for the cyan title screen (col=2, row=0)

```cpp
// sizeInSpritesheet passed to Sprite::createSprite
glm::vec2 TITLE_UV_SIZE   = glm::vec2(0.2500f, 0.5000f);

// keyframe offset passed to sprite->addKeyframe
glm::vec2 TITLE_UV_OFFSET = glm::vec2(0.5000f, 0.0000f);
```

---

## Task Breakdown

### TASK 1 — Update `Game.h`

**File:** `Game.h`

Add the following to the existing file:

1. Add a new `#include "Menu.h"` after `#include "Scene.h"`.
2. Add a `GameState` enum **before** the class definition:
```cpp
enum GameState { STATE_MENU, STATE_PLAYING, STATE_INSTRUCTIONS, STATE_CREDITS };
```
3. Add the following **public** method:
```cpp
void changeState(GameState newState);
```
4. Add the following **private** members:
```cpp
GameState currentState;
Menu menu;
```

The `Scene scene` and `bool keys[GLFW_KEY_LAST+1]` members already exist — do not duplicate them.

---

### TASK 2 — Rewrite `Game.cpp`

**File:** `Game.cpp`

Replace the entire file content with the following logic:

- `init()`: set `current
- STATE_MENU`, call `menu.init()`. Do NOT call `scene.init()` here.
- `update(deltaTime)`: dispatch to `menu.update(deltaTime)` or `scene.update(deltaTime)` based on `currentState`.
- `render()`: `glClear`, then dispatch to `menu.render()` or `scene.render()`.
- `keyPressed(key)`:
  - Always set `keys[key] = true`.
  - `GLFW_KEY_ESCAPE` → `bPlay = false`.
  - Dispatch `menu.keyPressed(key)` when `STATE_MENU`.
  - When `STATE_PLAYING`, handle cheats:
    - `GLFW_KEY_G` → `scene.toggleGodMode()`
    - `GLFW_KEY_K` → `scene.giveAllKeys()`
    - `GLFW_KEY_1`..`GLFW_KEY_5` → `scene.loadLevel(key - GLFW_KEY_1 + 1)`
- `keyReleased(key)`: set `keys[key] = false`.
- `changeState(newState)`: if `newState == STATE_PLAYING`, call `scene.init()` first. Then assign `currentState = newState`.

---

### TASK 3 — Create `Menu.h`

**File:** `Menu.h` (new file)

Define a class `Menu` with:

**Private members:**
```cpp
ShaderProgram texProgram;
glm::mat4 projection;

Texture  menuSheet;       // single spritesheet for all menu visuals
Sprite  *backgroundSprite; // shows the cyan title screen
Sprite  *cursorSprite;     // shows the cursor/arrow (reuses same sheet)

int selectedOption;
static const int NUM_OPTIONS = 3;
glm::vec2 optionPositions[NUM_OPTIONS];
```

**Public methods:**
```cpp
Menu();
~Menu();
void init();
void update(int deltaTime);
void render();
void keyPressed(int key);
```

**Private methods:**
```cpp
void initShaders();
```

Required includes: `<GL/glew.h>`, `<glm/glm.hpp>`, `"ShaderProgram.h"`, `"Texture.h"`, `"Sprite.h"`

---

### TASK 4 — Create `Menu.cpp`

**File:** `Menu.cpp` (new file)

Implement the `Menu` class:

**`init()`:**
1. Call `initShaders()`.
2. Set `projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f)`.
3. Set `selectedOption = 0`.
4. Load `menuSheet` from `"images/MainMenu.png"` with `TEXTURE_PIXEL_FORMAT_RGBA`.
   Also call:
   ```cpp
   menuSheet.setWrapS(GL_CLAMP_TO_EDGE);
   menuSheet.setWrapT(GL_CLAMP_TO_EDGE);
   menuSheet.setMinFilter(GL_NEAREST);
   menuSheet.setMagFilter(GL_NEAREST);
   ```
5. Create `backgroundSprite` using the precomputed UV values:
   ```cpp
   backgroundSprite = Sprite::createSprite(
       glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT), // quad fills the whole screen
       glm::vec2(0.2500f, 0.5000f),             // TITLE_UV_SIZE
       &menuSheet,
       &texProgram
   );
   backgroundSprite->setNumberAnimations(1);
   backgroundSprite->setAnimationSpeed(0, 1);
   backgroundSprite->addKeyframe(0, glm::vec2(0.5000f, 0.0000f)); // TITLE_UV_OFFSET
   backgroundSprite->changeAnimation(0);
   backgroundSprite->setPosition(glm::vec2(0.f, 0.f));
   ```
6. For `cursorSprite`: if a cursor sprite exists elsewhere in the sheet, use its UV.
   As a fallback, reuse the same sheet with a small region:
   ```cpp
   // Placeholder: use a 16x16 region — replace UV once you identify the cursor in the sheet
   cursorSprite = Sprite::createSprite(
       glm::ivec2(16, 16),
       glm::vec2(0.02f, 0.034f), // approx 16/779 x 10/294
       &menuSheet,
       &texProgram
   );
   cursorSprite->setNumberAnimations(1);
   cursorSprite->setAnimationSpeed(0, 1);
   cursorSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
   cursorSprite->changeAnimation(0);
   ```
7. Set `optionPositions`:
   ```cpp
   optionPositions[0] = glm::vec2(260.f, 280.f); // Jugar
   optionPositions[1] = glm::vec2(260.f, 320.f); // Instrucciones
   optionPositions[2] = glm::vec2(260.f, 360.f); // Créditos
   ```
   > ⚠️ Adjust these Y values once you see where the options land over the background image.

**`render()`:**
1. Call `texProgram.use()`.
2. Set uniforms: `"projection"` → `projection`, `"color"` → `(1,1,1,1)`, `"texCoordDispl"` → `(0,0)`, `"modelview"` → a local identity matrix variable (e.g. `glm::mat4 modelview(1.f)`).
3. Call `backgroundSprite->render()`.
4. Set cursor position to `optionPositions[selectedOption] - glm::vec2(24.f, 0.f)`.
5. Call `cursorSprite->setPosition(...)` then `cursorSprite->render()`.

**`keyPressed(key)`:**
- `GLFW_KEY_DOWN` → `selectedOption = (selectedOption + 1) % NUM_OPTIONS`
- `
- ` → `selectedOption = (selectedOption + NUM_OPTIONS - 1) % NUM_OPTIONS`
- `GLFW_KEY_ENTER` or `GLFW_KEY_KP_ENTER`:
  - option 0 → `Game::instance().changeState(STATE_PLAYING)`
  - option 1 → `Game::instance().changeState(STATE_INSTRUCTIONS)`
  - option 2 → `Game::instance().changeState(STATE_CREDITS)`

**`initShaders()`:**
- Mirror exactly what `Scene::initShaders()` does: load `"shaders/texture.vert"` and `"shaders/texture.frag"`, link, call `bindFragmentOutput("outColor")`.

**`~Menu()`:**
- `delete backgroundSprite` and `delete cursorSprite` if not nullptr.
- Call `texProgram.free()`.

---

### TASK 5 — Add stubs to `Scene.h` / `Scene.cpp`

**File:** `Scene.h`

Add the following **public** method declarations:
```cpp
void toggleGodMode();
void giveAllKeys();
void loadLevel(int levelNum);
```

**File:** `Scene.cpp`

Add stub implementations (empty body is fine for now):
```cpp
void Scene::toggleGodMode() { /* TODO */ }
void Scene::giveAllKeys()   { /* TODO */ }
void Scene::loadLevel(int levelNum) { /* TODO */ }
```

---

## Required Assets

Place the following file in the `images/` folder before running:

| File | Description |
|---|---|
| `images/MainMenu.png` | Original spritesheet (779×294 px) with all title screen variants — do NOT crop it |

---

## Compilation Checklist

Before building, verify:

- [ ] `Menu.h` and `Menu.cpp` are added to the Visual Studio project (or `CMakeLists.txt`)
- [ ] `#include "Menu.h"` is present in `Game.h`
- [ ] `images/MainMenu.png` exists (the full uncropped spritesheet)
- [ ] `Scene` has `toggleGodMode()`, `giveAllKeys()` and `loadLevel()` declared and defined

---

## Expected Behaviour After Implementation

1. Application starts → `STATE_MENU` → cyan title screen fills the screen.
2. Cursor appears next to "Jugar".
3. `↑` / `↓` moves cursor between the 3 options.
4. `Enter` on "Jugar" → `changeState(STATE_PLAYING)` → `scene.init()` runs → level loads normally.
5. `ESC` closes the application from any state.
