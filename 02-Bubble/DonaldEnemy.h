#ifndef _DONALD_ENEMY_INCLUDE
#define _DONALD_ENEMY_INCLUDE

#include "Enemy.h"

class DonaldEnemy : public Enemy
{
public:
	Type getType() const override { return Type::DONALD; }

protected:
	const char* getTextureFile() const override;
	glm::ivec2 getFrameSizePx() const override;
	void configureAnimations(float texW, float texH) override;
	void stepAI(int deltaTime, const glm::ivec2 &bugsTilePos) override;
	int getVisionRangeTiles() const override { return 12; }
	int getSearchDurationMs() const override { return 1400; }
private:
	void moveTowardTile(const glm::ivec2& targetTile, int stepPx = MOVE_STEP_PX);

};

#endif // _DONALD_ENEMY_INCLUDE