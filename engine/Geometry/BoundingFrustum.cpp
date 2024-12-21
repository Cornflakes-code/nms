#include "BoundingFrustum.h"

bool BoundingFrustum::intersects(const OWBounding* other) const
{
	return false;
}

bool BoundingFrustum::intersects(const AABB& box) const
{
	return topFace.isOnOrForwardPlane(box) && bottomFace.isOnOrForwardPlane(box) &&
		rightFace.isOnOrForwardPlane(box) && leftFace.isOnOrForwardPlane(box) &&
		farFace.isOnOrForwardPlane(box) && nearFace.isOnOrForwardPlane(box);
}


/*
constexpr float _max = std::numeric_limits<float>::max();

glm::vec4 Plane::ClosestPointOnPlane(const glm::vec4& p)
{
	// Not a bad site for beginner vectors
	// https://gdbooks.gitbooks.io/3dcollisions/content/Chapter1/closest_point_on_plane.html
	//// This works assuming plane.Normal is normalized, which it should be
	//float distance = DOT(plane.Normal, point) - plane.Distance;
	//// If the plane normal wasn't normalized, we'd need this:
	//// distance = distance / DOT(plane.Normal, plane.Normal);
	return glm::vec4(0.0);
}
*/
