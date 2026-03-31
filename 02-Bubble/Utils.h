#ifndef _UTILS_INCLUDE
#define _UTILS_INCLUDE

#include <glm/glm.hpp>

inline bool collidesWith(glm::vec2 posA, glm::ivec2 sizeA, glm::vec2 posB, glm::ivec2 sizeB)
{
	return posA.x < posB.x + sizeB.x &&
		posA.x + sizeA.x > posB.x &&
		posA.y < posB.y + sizeB.y &&
		posA.y + sizeA.y > posB.y;
}

#endif // _UTILS_INCLUDE
