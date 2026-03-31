#ifndef _FRANCO_ENEMY_INCLUDE
#define _FRANCO_ENEMY_INCLUDE

#include "Enemy.h"

class FrancoEnemy : public Enemy
{
public:
	Type getType() const override { return Type::FRANCO; }

protected:
	const char* getTextureFile() const override;
	glm::ivec2 getFrameSizePx() const override;
	void configureAnimations(float texW, float texH) override;
	void stepAI(int deltaTime, const glm::ivec2 &bugsTilePos) override;
	int getVisionRangeTiles() const override { return 14; }
	int getSearchDurationMs() const override { return 1800; }
};

#endif // _FRANCO_ENEMY_INCLUDE