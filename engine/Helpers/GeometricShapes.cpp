#include "GeometricShapes.h"

#include <glm/gtc/constants.hpp>

GeometricShapes::GeometricShapes()
{
}

std::vector<glm::vec2> GeometricShapes::circle(float radius, float arcRadians)
{
	std::vector<glm::vec2> retval;
	for (float i = 0.0f; i < glm::two_pi<float>(); i += arcRadians)
	{
		retval.push_back(glm::vec2(radius * glm::cos(i), radius * glm::sin(i)));
	}
	return retval;
}

std::vector<glm::vec2> GeometricShapes::torus(float innerRadius,
										float outerRadius,
										float arcRadians)
{
	std::vector<glm::vec2> retval;
	for (float i = 0.0f; i < glm::two_pi<float>(); i += arcRadians)
	{
		float arc = i + arcRadians;

		retval.push_back(glm::vec2(innerRadius * glm::cos(i), innerRadius * glm::sin(i)));
		retval.push_back(glm::vec2(outerRadius * glm::cos(i), outerRadius * glm::sin(i)));
		retval.push_back(glm::vec2(outerRadius * glm::cos(arc), outerRadius * glm::sin(arc)));

		retval.push_back(glm::vec2(innerRadius * glm::cos(i), innerRadius * glm::sin(i)));
		retval.push_back(glm::vec2(outerRadius * glm::cos(arc), outerRadius * glm::sin(arc)));
		retval.push_back(glm::vec2(innerRadius * glm::cos(arc), innerRadius * glm::sin(arc)));
	}
	return retval;
}

std::vector<glm::vec3> GeometricShapes::rectangle(const glm::vec2& dims, 
							const glm::vec2& bottomLeft)
{
	const float& x = bottomLeft.x;
	const float& y = bottomLeft.y;
	float z = 0.0f;
	std::vector<glm::vec3> retval;
	retval.push_back({ x, y, z });
	retval.push_back({ x, dims.y + y, z });
	retval.push_back({ dims.x + x, y, z });

	retval.push_back({ dims.x + x, y, z });
	retval.push_back({ x, dims.y + y, z });
	retval.push_back({ dims.x + x, dims.y + y, z });

	return retval;
}

// But the points are bunched near the poles.
std::vector<glm::vec3> GeometricShapes::pointsOnSphere(
	int numHoroSegments, int numVertSegments, float TAU)
{
	// https://www.youtube.com/watch?v=lctXaT9pxA0
	std::vector<glm::vec3> retVal;
	for (int h = 0; h < numHoroSegments; h++)
	{
		float angle1 = (h + 1) * glm::pi<float>() / (numHoroSegments + 1);
		for (int v = 0; v < numVertSegments; v++)
		{
			float angle2 = v * TAU / numVertSegments;
				float x = sin(angle1) * cos(angle2);
				float y = cos(angle1);
				float z = sin(angle1) * sin(angle2);
				glm::vec3 pointOnSphere = glm::vec3(x, y, z);
				retVal.push_back(pointOnSphere);
		}
	}
	return retVal;
}

// Evenly distributed points on a sphere.
std::vector<glm::vec3> GeometricShapes::fibonacciSphere(int numPoints, float TAU)
{
	// https://www.youtube.com/watch?v=lctXaT9pxA0

	// https://www.redblobgames.com/x/1842-delaunay-voronoi-sphere/
	std::vector<glm::vec3> retVal;

	const double goldenRatio = (1 + sqrt(5)) / 2.0;
	const double angleIncrement = TAU * goldenRatio;
	for (int i = 0; i < numPoints; i++)
	{
		double t = (i * 1.0) / numPoints;
		double angle1 = acos(1 - 2 * t);
		double angle2 = angleIncrement * i;
		float x = static_cast<float>(sin(angle1) * cos(angle2));
		float y = static_cast<float>(sin(angle1) * sin(angle2));
		float z = static_cast<float>(cos(angle1));
		glm::vec3 pointOnSphere = glm::vec3(x, y, z);
		retVal.push_back(pointOnSphere);
	}
	return retVal;
}

