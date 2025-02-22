#include "SceneGraphNode.h"
#include <map>

SceneGraphNode* SceneGraphNode::findChild(const std::string& _name)
{

	if (mName == _name)
		return this;
	for (SceneGraphNode* child : mChildren)
	{
		if (child->mName == _name)
			return child;
		SceneGraphNode* found = child->findChild(_name);
		if (found != nullptr)
			return found;
	}
	return nullptr;
}

SceneGraphNode* SceneGraphNode::addChild(SceneGraphNode* newChild)
{
	if (newChild->mParent)
	{
		throw NMSException("SceneGraphNode has a parent");
	}
	mChildren.push_back(newChild);
	newChild->mParent = this;
	return newChild;
}

void SceneGraphNode::render(const glm::mat4& proj,
	const glm::mat4& view, const glm::mat4& _model,
	const glm::vec3& cameraPos,
	RenderTypes::ShaderMutator renderCb,
	RenderTypes::ShaderResizer resizeCb)
{
	if (mName == "hsjaf")
	{
		mName = "hsjaf";
	}
	glm::mat4 model = glm::scale(_model, mScaleFactor);
	model = glm::translate(model, mTranslateVector);
	if (!readyForRender() && mParent != nullptr)
	{
		throw NMSLogicException("Actor: " + mName + " not ready for render.");
	}
	for (SceneGraphNode* child : mChildren)
	{
		child->render(proj, view, model, cameraPos,
			renderCb, resizeCb);
	}
}

bool SceneGraphNode::traverse(SceneGraphNodeCallbackType proc)
{
	if (!proc(this))
		return false;
	for (SceneGraphNode* child : mChildren)
	{
		if (!child->traverse(proc))
			return false;
	}
	return true;
}
