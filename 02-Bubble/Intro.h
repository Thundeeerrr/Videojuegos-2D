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
    Sprite  *slide;

    float alpha;
    float timer;
    bool  finished;

    enum Phase { FADE_IN, HOLD, FADE_OUT } phase;

    static constexpr float FADE_IN_TIME  = 1.2f;
    static constexpr float HOLD_TIME     = 3.0f;
    static constexpr float FADE_OUT_TIME = 1.0f;
};

#endif // _INTRO_INCLUDE
