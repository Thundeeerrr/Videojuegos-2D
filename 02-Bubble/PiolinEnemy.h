#ifndef _PIOLIN_ENEMY_INCLUDE
#define _PIOLIN_ENEMY_INCLUDE

#include "Enemy.h"

class PiolinEnemy : public Enemy
{
public:
	Type getType() const override { return Type::PIOLIN; }

protected:
	const char* getTextureFile() const override;
	glm::ivec2 getFrameSizePx() const override;
	void configureAnimations(float texW, float texH) override;
	void stepAI(int deltaTime, const glm::ivec2 &bugsTilePos) override;
	int getVisionRangeTiles() const override { return 8; }
};

#endif // _PIOLIN_ENEMY_INCLUDE