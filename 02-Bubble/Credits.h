#ifndef _CREDITS_INCLUDE
#define _CREDITS_INCLUDE

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "Texture.h"
#include "Sprite.h"

class Credits {
public:
    Credits();
    ~Credits();
    void init();
    void update(int deltaTime);
    void render();
    bool isFinished() const { return finished; }
    void skip() { finished = true; }

private:
    void initShaders();

    ShaderProgram texProgram;
    glm::mat4     projection;

    Texture sheet;
    Sprite *slide;
    bool finished;
    int elapsedTimeMs;
    static const int DURATION_MS = 5000;
};

#endif // _CREDITS_INCLUDE
