#include "OWActor.h"

void OWActor::begin()
{
}

void OWActor::tick(float deltaSecods)
{
}

void OWActor::end()
{
}

void OWActor::destroy()
{
}


void OWActor::render(const glm::mat4& proj,
	const glm::mat4& view, const glm::mat4& _model,
	const glm::vec3& cameraPos,
	RenderTypes::ShaderMutator renderCb,
	RenderTypes::ShaderResizer resizeCb)
{
	std::string s = name();
	if (s == "stars")
		s = "stars";
	if (s == "Text: Enjoy it while you can.")
	{
		s = "Text: Enjoy it while you can.";
	}
	for (OWSceneComponent* c : mSceneComponents)
	{
		c->render(proj, view, _model, cameraPos,
			renderCb, resizeCb);
	}
}
