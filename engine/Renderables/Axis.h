#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "../OWEngine/OWEngine.h"

#include "../Renderables/Renderer.h"
#include "../Renderables/BoundingBox.h"
#include "SimpleVertexRender.h"

class TextBillboard;
class Points;
class Shader;
class Camera;
/*
	Wraps the rendering of an XYZ set of labelled axis. Useful for early development
*/
class OWENGINE_API Axis: public Renderer
{
public:
	Axis(Shader* _shader = nullptr);

	void setUp(const AABB& world, const Camera* camera);
	void render(const glm::mat4& proj, const glm::mat4& view, 
				const glm::mat4& model) const;
private:
#pragma warning( push )
#pragma warning( disable : 4251 )
	SimpleVertexRender mX;
	SimpleVertexRender mY;
	SimpleVertexRender mZ;
	SimpleVertexRender mCircle;
	Points* mPoints;
	std::vector<glm::vec3> mLines;
#pragma warning( pop )
};