#include "Model.h"

#include <glm/ext/matrix_transform.hpp>

#include "../Helpers/MoveController.h"
#include "../Helpers/ErrorHandling.h"

#include "ModelRenderer.h"

void Model::addRenderer(ModelRenderer* source)
{
	mRenderer = source;
	mRenderer->prepare(this);
}

void Model::render(const glm::mat4& proj,
	const glm::mat4& view,
	const glm::mat4& model,
	const MoveController* mover,
	OWUtils::RenderCallbackType renderCb,
	OWUtils::ResizeCallbackType resizeCb) const
{
	if (mover)
	{
		mRenderer->render(this, proj, view, mover->translate(model), renderCb, resizeCb);
	}
	else
	{
//		glm::mat4 initialPositionModel = glm::translate(model, initialPosition());
//		mRenderer->render(this, proj, view, initialPositionModel, renderCb, resizeCb);
	}
}