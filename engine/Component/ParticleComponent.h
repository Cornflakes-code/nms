#pragma once

#include <glm/glm.hpp>

#include "../OWEngine/OWEngine.h"
#include "../Component/ShapeComponent.h"

class OWSphere;
class AABB;

struct OWENGINE_API ParticleComponentData: public ShapeComponentData
{
};

class OWENGINE_API ParticleComponent: public ShapeComponent
{
	// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html

	// Complete Tutorial for particles
//https://www.team-nutshell.dev/nutshellengine/articles/particle-rendering-2.html
//https://www.team-nutshell.dev/nutshellengine/articles/particle-rendering.html
#pragma warning( push )
#pragma warning( disable : 4251 )
	OWSphere* mSphere;
#pragma warning( pop )
public:
	ParticleComponent(OLDActor* _owner, ParticleComponentData* _data);
	// returns true if ray goes through box else false Populates normal vector with 
	// distance is length of ray until intersection
	// distance is -ve if the AABB is behind this
	//bool intersects(const AABB& box, glm::vec3& normal, float& distance) const;
	bool intersects(const AABB& box) const override;
	void doInit() override;
};
