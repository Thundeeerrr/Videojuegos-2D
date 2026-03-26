# Task: Implement Credits & Intro Splash Screens with Fade Animations

## Context

This task adds two new screens to the existing state machine:

- **`STATE_INTRO`** — a splash/logo screen shown **before** the menu, on app launch.
- **`STATE_CREDITS`** — shown when the player selects "Créditos" from the menu.

Both screens use **fade-in → hold → fade-out** transitions driven by a `float alpha` passed to the existing `"color"` uniform (`vec4`) in `texture.frag`. **No shader changes are needed.** The alpha blending is handled by enabling `GL_BLEND` once in `Game`.

The spritesheet pixel coordinates (`leftX`, `rightX`, `topY`, `bottomY`) are **intentionally left as `TODO` placeholders** — fill them in after measuring your PNG manually.

---

## Existing Files (DO NOT modify unless instructed)

| File | Role |
|---|---|
| `Game.h / Game.cpp` | Singleton, state machine — **will be modified** |
| `Menu.h / Menu.cpp` | Already implemented menu — do not touch |
| `ShaderProgram.h/.cpp` | Exposes `setUniform4f` — the alpha channel of `"color"` drives the fade |
| `Texture.h/.cpp` | `loadFromFile(path, TEXTURE_PIXEL_FORMAT_RGBA)` |
| `Sprite.h/.cpp` | Quad VAO/VBO + keyframe animations |
| `shaders/texture.vert/.frag` | Already consume `uniform vec4 color` — no changes needed |

### UV Helper Function

Both new `.cpp` files must include this function (copy from `Menu.cpp`):

```cpp
glm::vec4 calculateUV(float leftX, float rightX, float topY, float bottomY,
                      float IMG_W, float IMG_H)
{
    return glm::vec4(
        (rightX - leftX) / IMG_W,   // uvSizeX
        (bottomY - topY) / IMG_H,   // uvSizeY
        leftX / IMG_W,              // uvOffsetX
        topY  / IMG_H               // uvOffsetY
    );
}
```

---

## Fade Logic Reference

All fade screens share the same state machine. Use this as the canonical pattern:

```
FADE_IN  →  HOLD  →  FADE_OUT  →  finished = true
```

| Phase | Alpha formula | Advance when |
|-------|--------------|-------------|
| `FADE_IN` | `alpha = timer / fadeInTime` (clamp to 1) | `timer >= fadeInTime` |
| `HOLD` | `alpha = 1.f` (constant) | `timer >= holdTime` |
| `FADE_OUT` | `alpha = 1.f - timer / fadeOutTime` (clamp to 0) | `timer >= fadeOutTime` |

`timer` resets to `0.f` on every phase transition.  
`deltaTime` is in **milliseconds** → convert with `float dt = deltaTime / 1000.f`.

---

## TASK 1 — Enable Alpha Blending in `Game.cpp`

**File:** `Game.cpp` → inside `Game::init()`, **after** `glClearColor` and **before** any state init:

```cpp
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

> ⚠️ This must be called **once globally**. Do not enable/disable it per-frame inside render methods.

---

## TASK 2 — Add States to `Game.h`

**File:** `Game.h`

1. Add `STATE_INTRO` and `STATE_CREDITS` to the existing `GameState` enum:
```cpp
enum GameState { STATE_INTRO, STATE_MENU, STATE_PLAYING, STATE_INSTRUCTIONS, STATE_CREDITS };
```

2. Add includes and members:
```cpp
#include "Intro.h"
#include "Credits.h"
```

3. Add private members (alongside existing `Menu menu` and `Scene scene`):
```cpp
Intro   intro;
Credits credits;
```

---

## TASK 3 — Update `Game.cpp` Dispatch

**File:** `Game.cpp`

**`init()`** — change starting state to `STATE_INTRO` and init both new screens:
```cpp
intro.init();
credits.init();
currentState = STATE_INTRO;
// Do NOT call menu.init() or scene.init() here
```

**`update(deltaTime)`** — add cases:
```cpp
case STATE_INTRO:
    intro.update(deltaTime);
    if (intro.isFinished())
        changeState(STATE_CREDITS);
    break;
case STATE_CREDITS:
    credits.update(deltaTime);
    if (credits.isFinished())
        changeState(STATE_MENU);
    break;
```

**`render()`** — add cases:
```cpp
case STATE_INTRO:   intro.render();   break;
case STATE_CREDITS: credits.render(); break;
```

**`changeState(newState)`** — add reset calls so screens can be replayed:
```cpp
void Game::changeState(GameState newState) {
    if (newState == STATE_PLAYING)  scene.init();
    if (newState == STATE_INTRO)    intro.init();
    if (newState == STATE_CREDITS)  credits.init();
    if (newState == STATE_MENU)     menu.init();
    currentState = newState;
}
```

> Calling `init()` on state entry resets `timer`, `alpha`, and `phase` automatically — no separate reset method needed.

---

## TASK 4 — Create `Intro.h`

**File:** `Intro.h` (new file)

```cpp
#ifndef _INTRO_INCLUDE
#define _INTRO_INCLUDE

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "Texture.h"
#include "Sprite.h"

class Intro {
public:
    Intro();
    ~Intro();
    void init();
    void update(int deltaTime);
    void render();
    bool isFinished() const { return finished; }
    void skip() { alpha = 0.f; finished = true; }

private:
    void initShaders();

    ShaderProgram texProgram;
    glm::mat4     projection;

    Texture  sheet;
    Sprite  *slide;         // single full-screen image

    float alpha;
    float timer;
    bool  finished;

    enum Phase { FADE_IN, HOLD, FADE_OUT } phase;

    static constexpr float FADE_IN_TIME  = 1.2f; // seconds
    static constexpr float HOLD_TIME     = 3.0f;
    static constexpr float FADE_OUT_TIME = 1.0f;
};

#endif
```

---

## TASK 5 — Create `Intro.cpp`

**File:** `Intro.cpp` (new file)

**Constructor / Destructor:**
```cpp
Intro::Intro() : slide(nullptr), alpha(0.f), timer(0.f),
                 finished(false), phase(FADE_IN) {}

Intro::~Intro() {
    if (slide != nullptr) delete slide;
    texProgram.free();
}
```

**`init()`:**
```cpp
void Intro::init() {
    initShaders();
    projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
    alpha    = 0.f;
    timer    = 0.f;
    finished = false;
    phase    = FADE_IN;

    sheet.loadFromFile("images/MainMenu.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sheet.setWrapS(GL_CLAMP_TO_EDGE);
    sheet.setWrapT(GL_CLAMP_TO_EDGE);
    sheet.setMinFilter(GL_NEAREST);
    sheet.setMagFilter(GL_NEAREST);

    // TODO: replace these pixel values after measuring your spritesheet
    const float IMG_W   = 779.f, IMG_H = 294.f;
    const float LEFT_X  = 0.f;   // TODO
    const float RIGHT_X = 194.f; // TODO
    const float TOP_Y   = 0.f;   // TODO
    const float BOT_Y   = 147.f; // TODO
    glm::vec4 uv = calculateUV(LEFT_X, RIGHT_X, TOP_Y, BOT_Y, IMG_W, IMG_H);

    slide = Sprite::createSprite(
        glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT),
        glm::vec2(uv.x, uv.y),
        &sheet, &texProgram
    );
    slide->setNumberAnimations(1);
    slide->setAnimationSpeed(0, 1);
    slide->addKeyframe(0, glm::vec2(uv.z, uv.w));
    slide->changeAnimation(0);
    slide->setPosition(glm::vec2(0.f, 0.f));
}
```

**`update(int deltaTime)`:**
```cpp
void Intro::update(int deltaTime) {
    if (finished) return;
    float dt = deltaTime / 1000.f;
    timer += dt;

    switch (phase) {
        case FADE_IN:
            alpha = glm::clamp(timer / FADE_IN_TIME, 0.f, 1.f);
            if (timer >= FADE_IN_TIME) { alpha = 1.f; phase = HOLD; timer = 0.f; }
            break;
        case HOLD:
            if (timer >= HOLD_TIME) { phase = FADE_OUT; timer = 0.f; }
            break;
        case FADE_OUT:
            alpha = glm::clamp(1.f - timer / FADE_OUT_TIME, 0.f, 1.f);
            if (timer >= FADE_OUT_TIME) { alpha = 0.f; finished = true; }
            break;
    }
}
```

**`render()`:**
```cpp
void Intro::render() {
    if (finished) return;
    glm::mat4 modelview(1.f);
    texProgram.use();
    texProgram.setUniformMatrix4f("projection", projection);
    texProgram.setUniform4f("color", 1.f, 1.f, 1.f, alpha);  // alpha drives fade
    texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
    texProgram.setUniformMatrix4f("modelview", modelview);
    slide->render();
}
```

**`initShaders()`:** copy exactly from `Menu::initShaders()` — same shaders, same `bindFragmentOutput("outColor")`.

---

## TASK 6 — Create `Credits.h`

**File:** `Credits.h` (new file)

Identical structure to `Intro.h` with these differences:

```cpp
// Different timing constants
static constexpr float FADE_IN_TIME  = 1.5f;
static constexpr float HOLD_TIME     = 5.0f;
static constexpr float FADE_OUT_TIME = 1.5f;
```

Everything else (members, methods, enum) is the same as `Intro.h` — change class name to `Credits` and include guard to `_CREDITS_INCLUDE`.

---

## TASK 7 — Create `Credits.cpp`

**File:** `Credits.cpp` (new file)

Identical implementation to `Intro.cpp` with these differences:

1. Class name: `Credits` instead of `Intro`.
2. The `TODO` pixel coordinates point to the **credits region** of your spritesheet:
```cpp
// TODO: replace with credits screen pixel coordinates from your spritesheet
const float LEFT_X  = 0.f;   // TODO
const float RIGHT_X = 194.f; // TODO
const float TOP_Y   = 0.f;   // TODO
const float BOT_Y   = 147.f; // TODO
```
3. Timing constants will match those declared in `Credits.h`.

---

## TASK 8 — Allow Skipping with Any Key

**File:** `Game.cpp` → `keyPressed(key)` — add cases:

```cpp
case STATE_INTRO:
    intro.skip();
    break;
case STATE_CREDITS:
    credits.skip();
    break;
```

The `skip()` method is already declared in both `Intro.h` and `Credits.h` (added in Tasks 4 and 6).

---

## Compilation Checklist

- [x] `Intro.h` / `Intro.cpp` added to project (`.vcxproj` or `CMakeLists.txt`)
- [x] `Credits.h` / `Credits.cpp` added to project
- [x] `#include "Intro.h"` and `#include "Credits.h"` present in `Game.h`
- [x] `STATE_INTRO` and `STATE_CREDITS` added to `GameState` enum
- [x] `glEnable(GL_BLEND)` called in `Game::init()` before any `init()` call
- [x] `images/MainMenu.png` exists and is the full uncropped spritesheet
- [ ] All `TODO` UV coordinates filled in with measured pixel values

---

## Expected Behaviour After Implementation

1. App starts → `STATE_INTRO` → screen fades in from black → holds → fades out.
2. Auto-transitions to `STATE_CREDITS` (or player presses any key to skip intro).
3. `STATE_CREDITS` plays fade in/hold/fade out.
4. Credits finish → auto-returns to `STATE_MENU` (or player skips credits with any key).
5. `ESC` closes from any state.
