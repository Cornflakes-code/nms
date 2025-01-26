#pragma once

#include "../Core/OWObject.h"
#include <vector>

#include "../OWEngine/OWEngine.h"
#include "../Scripting/OWActorScript.h"
#include "../Component/OWComponent.h"
#include "../Component/OWMovableComponent.h"
#include "../Component/OWSceneComponent.h"
#include "../Renderers/OWRenderable.h"

class Scene;
class OWENGINE_API OWActor: public OWObject, public OWGameIFace, public OWIRenderable
{
	Scene* mScene;
	OWActorScript* mScript;
	std::vector<OWSceneComponent*> mSceneComponents;
	std::vector<OWIRenderable*> mComponents;
protected:
	virtual OWActorScript* script()
	{
		return mScript;
	}
public:
	OWActor(Scene* _scene, OWActorScript* _script)
		: mScene(_scene), mScript(_script)
	{
	}
	virtual const OWActorScript* script() const
	{
		return mScript;
	}
	virtual void addSceneComponent(OWSceneComponent* c) { mSceneComponents.push_back(c); }
	bool collideHandled(OWIMovable* OW_UNUSED(_ourComponent), OWActor* OW_UNUSED(other), OWIMovable* OW_UNUSED(otherComponent))
	{
		// returning true means we have dealt with it
		// returning false lets _ourComponent deal with it. I assume they just rebound.
		return false;
	}
	virtual bool canCollide(OWIMovable* OW_UNUSED(_ourComponent), OWActor* OW_UNUSED(other), OWIMovable* OW_UNUSED(otherComponent))
	{
		// for example our thigh and shin of same leg may interesect but they cannot collide
		// but hands of different arms can.
		return true;
	}
protected:
	void doInit() override;
public:
	void begin() override;
	void tick(float deltaSecods) override;
	void end() override;
	void destroy() override;

	void render(const glm::mat4& proj,
		const glm::mat4& view,
		const glm::mat4& model,
		const glm::vec3& cameraPos,
		RenderTypes::ShaderMutator renderCb = nullptr,
		RenderTypes::ShaderResizer resizeCb = nullptr) override;
};
