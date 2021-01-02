#pragma once
#include <vector>

#include <glm/glm.hpp>

#include "../OWEngine/OWEngine.h"

#include "Vertices.h"

class ParticlesRenderer;

class OWENGINE_API Particles : public Vertices
{
public:
	Particles();
	void renderer(ParticlesRenderer* source);
	void positions(const std::vector<glm::vec3>& _positions,
					unsigned int location,
					unsigned int divisor,
					unsigned int mode);
	void positions(const std::vector<glm::vec4>& _positions,
					unsigned int location,
					unsigned int divisor,
					unsigned int mode);
	void colours(const std::vector<glm::vec4>& _colours,
					unsigned int location,
					unsigned int colourDivisor = GL_INVALID_INDEX);
	virtual void render(const glm::mat4& proj,
					const glm::mat4& view,
					const glm::mat4& model,
					const MoveController* mover = nullptr,
					OWUtils::RenderCallbackType renderCb = nullptr,
					OWUtils::ResizeCallbackType resizeCb = nullptr) const;
private:
#pragma warning( push )
#pragma warning( disable : 4251 )
	ParticlesRenderer* mRenderer = nullptr;
	std::vector<glm::vec3> mPositionsV3;
	std::vector<glm::vec4> mPositionsV4;
	std::vector<glm::vec4> mColours;
	unsigned int mPositionMode = GL_INVALID_ENUM;
	unsigned int mPositionDivisor = GL_INVALID_INDEX;
	unsigned int mColourDivisor = GL_INVALID_INDEX;
	unsigned int mPositionLocation = GL_INVALID_INDEX;
	unsigned int mColourLocation = GL_INVALID_INDEX;

	friend class ParticlesRenderer;
#pragma warning( pop )
};