#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "../OWEngine/OWEngine.h"

#include "../Core/BoundingBox.h"

class OWENGINE_API MoveController
{
public:
	MoveController();
	void targetGeometry(const AABBV3& targetBounds, const glm::vec3& initialPosition);
	void move(const glm::vec3& speed);
	void direction(const glm::vec3& newValue) { mDirection = newValue; }
	void setPosition(const glm::vec3& newValue);
	void bounceIfCollide(const AABBV3& scenery);
	glm::vec3 translateVector() const
	{
		return glm::vec3(mBounds.center() - mInitialPosition);
	}
private:
#pragma warning( push )
#pragma warning( disable : 4251 )
	Compass::Direction wallIntersection(const AABBV3& scenery);
	glm::vec3 mDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 mInitialPosition;
	AABBV3 mBounds;
#pragma warning( pop )
};