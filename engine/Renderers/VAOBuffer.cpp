#include "VAOBuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Core/CommonUtils.h"
#include "../Core/ErrorHandling.h"

#include "../Helpers/Shader.h"

/*
* Very Modern OpenGL
https://www.reddit.com/r/opengl/comments/18rkgg3/one_vao_for_multiple_vbos/
https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions#glbuffer
*/
VAOBuffer::VAOBuffer(Shader* shader)
	: RendererBase(shader)
{
//	enum RenderType { DRAW_NONE, DRAW_ARRAYS, DRAW_MULTI, DRAW_PRIMITIVE };
//	mDrawType = DRAW_PRIMITIVE;
	mDrawType = DRAW_MULTI;
	mDrawType = DRAW_ARRAYS;
}

void VAOBuffer::add(const MeshDataLight* meshData)
{
	const float* ff = nullptr;
	validate(meshData);
	MeshDataLight::RenderData rd = meshData->mRenderData;
	if (!meshData->mVec3.empty())
	{
		mVec3.insert(mVec3.end(), meshData->mVec3.begin(), meshData->mVec3.end());
		if (mDrawType == RenderType::DRAW_PRIMITIVE)
		{
			int begin = mPrimitiveIndices.size();
			for (int i = 0; i < meshData->mVec3.size(); i++)
				mPrimitiveIndices.push_back(i + begin);
			mPrimitiveIndices.push_back(mPrimitiveRestart);
		}
		rd.verticesCount = meshData->mVec3.size();
	}
	else if (!meshData->mVec4.empty())
	{
		mVec4.insert(mVec4.end(), meshData->mVec4.begin(), meshData->mVec4.end());
		if (mDrawType == RenderType::DRAW_PRIMITIVE)
		{
			std::vector<int> temp = std::vector<int>(meshData->mVec4.size(), 7);
			mPrimitiveIndices.insert(mPrimitiveIndices.end(), temp.begin(), temp.end());
			mPrimitiveIndices.push_back(mPrimitiveRestart);
		}
		rd.verticesCount = meshData->mVec4.size();
	}
	if (!meshData->mIndices.empty())
	{
		mIndices.insert(mIndices.end(), meshData->mIndices.begin(), meshData->mIndices.end());
		rd.indicesCount = sizeof(unsigned int) * static_cast<GLsizei>(meshData->mIndices.size());
	}
	mData.push_back(rd);
}

void VAOBuffer::add(const std::vector<glm::vec3>& v,
	unsigned int vertexMode, unsigned int vertexLocation)
{
	MeshDataLight mdl;
	mdl.mVec3 = v;
	mdl.mRenderData.verticesCount = v.size();
	mdl.mRenderData.indicesCount = 0;
	mdl.mRenderData.indicesMode = 0;
	mdl.mRenderData.vertexMode = vertexMode;
	mdl.mRenderData.vertexLocation = vertexLocation;
	add(&mdl);
}

void VAOBuffer::add(const std::vector<glm::vec4>& v,
	unsigned int vertexMode, unsigned int vertexLocation)
{
	MeshDataLight mdl;
	mdl.mVec4 = v;
	mdl.mRenderData.verticesCount = v.size();
	mdl.mRenderData.indicesCount = 0;
	mdl.mRenderData.indicesMode = 0;
	mdl.mRenderData.vertexMode = vertexMode;
	mdl.mRenderData.vertexLocation = vertexLocation;
	add(&mdl);
}

void VAOBuffer::addLightSource(RendererBase* aLight)
{
}

void VAOBuffer::prepare()
{
	if (mVec4.size() && mVec3.size())
		throw NMSLogicException("VAOBuffer::prepare cannot have both vec4 and vec3 arrays.");
	if (mVec4.size() == 0 && mVec3.size() == 0)
		return;
	validateBase();
	if (mVao == std::numeric_limits<unsigned int>::max())
		glGenVertexArrays(1, &mVao);
	glBindVertexArray(mVao);

	glGenBuffers(1, &mVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mVbo);
	shader()->use();
	if (!mData[0].shaderColourName.empty())
	{
		shader()->setVector4f(mData[0].shaderColourName, mData[0].colour);
	}

	if (!mIndices.empty())
	{
		glGenBuffers(1, &mEbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(unsigned int) * static_cast<GLsizei>(mIndices.size()),
			mIndices.data(), GL_STATIC_DRAW);
	}
	if (mVec4.size())
	{
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(0); //mData.vertexLocation);
		glBufferData(GL_ARRAY_BUFFER, mVec4.size() * 4 * sizeof(float), mVec4.data(), GL_STATIC_DRAW);
	}
	else
	{
		// Youtube vidioe explaning glVertexAttribPointer
		// https://www.youtube.com/watch?v=kQOwkG15dYo
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0); //mData.vertexLocation);
		glBufferData(GL_ARRAY_BUFFER, mVec3.size() * 3 * sizeof(float), mVec3.data(), GL_STATIC_DRAW);
	}
	if (mDrawType == RenderType::DRAW_PRIMITIVE)
	{
		glGenBuffers(1, &mPrimitiveEbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mPrimitiveEbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mPrimitiveIndices.size(), mPrimitiveIndices.data(), GL_STATIC_DRAW);
	}

	glEnableVertexAttribArray(0);
	if (mDrawType == RenderType::DRAW_MULTI)
	{
		GLint startIndex = 0;
		for (MeshDataLight::RenderData rd : mData)
		{
			mMultiArrayStartIndexes.push_back(startIndex);
			startIndex += rd.verticesCount;
			mMultiArrayVertexCount.push_back(rd.verticesCount);
		}
	}

	// You can unbind the VAO afterwards so other VAO calls won't 
	// accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't 
	// unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);
}

void VAOBuffer::doRender() const
{
	// Look at:
	// https://www.reddit.com/r/opengl/comments/19bgtcb/is_the_effort_of_glmultidrawelements_worth_it/
	if (!mData[0].shaderColourName.empty())
	{
		shader()->setVector4f(mData[0].shaderColourName, mData[0].colour);
	}

	glBindVertexArray(mVao);
	if (mDrawType == RenderType::DRAW_PRIMITIVE)
	{
		// FPS = 27/28
		glBindBuffer(GL_ARRAY_BUFFER, mVbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mPrimitiveEbo);
		checkGLError();
		glDrawElements(mData[0].vertexMode, mPrimitiveIndices.size(), GL_UNSIGNED_INT, 0);
		checkGLError();
	}
	if (mDrawType == RenderType::DRAW_ARRAYS)
	{
		// FPS = 26/27
		GLsizei sum = 0;
		for (const MeshDataLight::RenderData& rd : mData)
		{
			if (mIndices.size())
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);
				glDrawElements(rd.indicesMode,
					static_cast<GLsizei>(rd.indicesCount),
					GL_UNSIGNED_INT, 0);
			}
			else
			{
				glDrawArrays(rd.vertexMode, sum, static_cast<GLsizei>(rd.verticesCount));
				sum += rd.verticesCount;
			}
		}
	}
	if (mDrawType == RenderType::DRAW_MULTI)
	{
		// FPS = 27/28
		glMultiDrawArrays(mData[0].vertexMode, mMultiArrayStartIndexes.data(), mMultiArrayVertexCount.data(), mData.size());
	}
	glBindVertexArray(0);
}

void VAOBuffer::validate(const MeshDataLight* mdl)
{
	validateBase();
	if (mdl->mVec4.empty() == mdl->mVec3.empty())
	{
		if (mdl->mVec4.empty())
			throw NMSLogicException("LightRenderer has no data");
		else
			throw NMSLogicException("LightRenderer has v3 and v4 data");
	}
}
