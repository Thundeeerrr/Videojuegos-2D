#ifndef _FRANCO_ENEMY_INCLUDE
#define _FRANCO_ENEMY_INCLUDE

#include "Enemy.h"

class FrancoEnemy : public Enemy
{
public:
	Type getType() const override { return Type::FRANCO; }
	void enforceHorizontalPatrolRange(float minX, float maxX);
	bool consumePendingShot(int &dirSign);

protected:
	const char* getTextureFile() const override;
	glm::ivec2 getFrameSizePx() const override;
	void configureAnimations(float texW, float texH) override;
	void stepAI(int deltaTime, const glm::ivec2 &bugsTilePos) override;
 int getVisionRangeTiles() const override { return 8; }
    int getAnimationCount() const override { return 6; }
	int getStandAnimationId() const override { return facingRight ? STAND : STAND_LEFT; }
	int getWalkAnimationId() const override { return facingRight ? WALK : WALK_LEFT; }
  bool getForcedAnimationId(int &animId) const override;
	bool shouldMirrorWhenFacingLeft() const override { return false; }

private:
    enum DirectionalAnims { STAND_LEFT = 2, WALK_LEFT = 3, SHOOT = 4, SHOOT_LEFT = 5 };
	static const int SHOOT_INTERVAL_MS = 2200;
	static const int SHOOT_FPS = 12;
	static const int SHOOT_FRAMES = 4;
	static const int SHOOT_ANIM_DURATION_MS = (SHOOT_FRAMES * 1000) / SHOOT_FPS;
	int shootCooldownMs = SHOOT_INTERVAL_MS;
	int shootAnimRemainingMs = 0;
  int pendingShotDir = 0;
  bool hasPatrolBaseY = false;
	int patrolBaseY = 0;
};

#endif // _FRANCO_ENEMY_INCLUDE