#include "Particles.h"
#include <cmath>
#include <glm/ext/matrix_transform.hpp>

#include "../Helpers/MoveController.h"
#include "../Helpers/ErrorHandling.h"

#include "ParticlesRenderer.h"

Particles::Particles()
:Vertices(glm::vec3(0.0f, 0.0f, 0.0f))
{
}

void Particles::renderer(ParticlesRenderer* source)
{
	mRenderer = source;
	mRenderer->prepare(this);
}

void Particles::positions(const std::vector<glm::vec3>& _positions,
	unsigned int location,
	unsigned int divisor,
	unsigned int mode)
{
	mPositionsV3 = _positions;
	mPositionLocation = location;
	mPositionDivisor = divisor;
	mPositionMode = mode;
}

void Particles::positions(const std::vector<glm::vec4>& _positions,
	unsigned int location,
	unsigned int divisor,
	unsigned int mode)
{
	mPositionsV4 = _positions;
	mPositionLocation = location;
	mPositionDivisor = divisor;
	mPositionMode = mode;
}

void Particles::colours(const std::vector<glm::vec4>& _colours,
							unsigned int location,
							unsigned int colourDivisor)
{
	mColours = _colours;
	mColourLocation = location;
	mColourDivisor = colourDivisor;
	if (mPositionsV3.size() == 0 && mPositionsV4.size() == 0)
	{
		throw NMSException("Particles.colours needs number of positions set to calc divisor");
	}
	else if (mPositionsV3.size())
	{
		if (colourDivisor == GL_INVALID_INDEX)
		{
			mColourDivisor = static_cast<unsigned int>(
				std::ceil(mPositionsV3.size() / (1.0f * mColours.size())));
		}
	}
	else if (mPositionsV4.size())
	{
		if (colourDivisor == GL_INVALID_INDEX)
		{
			mColourDivisor = static_cast<unsigned int>(
				std::ceil(mPositionsV4.size() / (1.0f * mColours.size())));
		}
	}
}

void Particles::render(const glm::mat4& proj,
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
		glm::mat4 initialPositionModel = glm::translate(model, initialPosition());
		mRenderer->render(this, proj, view, initialPositionModel, renderCb, resizeCb);
	}
}
