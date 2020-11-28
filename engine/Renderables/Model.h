#pragma once

#include <vector>
#include <functional>


#include <glm/glm.hpp>

#ifndef __gl_h_
#include <glad/glad.h>
#endif

#include "../OWEngine/OWEngine.h"
#include "../Helpers/CommonUtils.h"
#include "RenderableBase.h"

class Shader;
class ModelRenderer;
/*
	Basically a struct that contains the data to be rendered.
	Passed to VertexRenderer to be rendered.
	Can be used as is or as a base class for more complex graphics.
	No getters. Values only accessible by VertexRenderer and derived classes.
	Does this means it cannot be inherited from outside of the DLL?
*/
class OWENGINE_API Model : public RenderableBase
{
public:
	Model() {}
	void addRenderer(ModelRenderer* source);
	void render(const glm::mat4& proj,
		const glm::mat4& view,
		const glm::mat4& model,
		const MoveController* mover = nullptr,
		OWUtils::RenderCallbackType renderCb = nullptr,
		OWUtils::ResizeCallbackType resizeCb = nullptr) const override;

	void indices(const std::vector<unsigned int>& newValue,
		unsigned int drawMode)
	{
		mIndices = newValue;
		mIndicesMode = drawMode;
	}
	void texture(const OWUtils::TextureBlock& tb)
	{
		// If there are more than one texture unit in a shader then we need to consider
		// GL_TEXTURE1, GL_TEXTURE2 and so on. See:
		// https://opentk.net/learn/chapter1/5-multiple-textures.html
		mTextures.push_back(tb);
	}
protected:
#pragma warning( push )
#pragma warning( disable : 4251 )
	ModelRenderer* mRenderer = nullptr;
	std::vector<unsigned int> mIndices;
	std::vector<OWUtils::TextureBlock> mTextures;
	glm::vec4 mColour;
	std::string mColourName;
	unsigned int mIndicesMode = GL_INVALID_ENUM;
	friend class ModelRenderer;
#pragma warning( pop )
};
