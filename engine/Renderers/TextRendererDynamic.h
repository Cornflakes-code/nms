#pragma once
#include <vector>

#ifndef __gl_h_
#include <glad/glad.h>
#endif

#include "../OWEngine/OWEngine.h"

#include "TextRenderer.h"

class TextComponent;

class OWENGINE_API TextRendererDynamic: public TextRenderer
{
public:
	TextRendererDynamic();
protected:
	void doSetup(const TextComponent* td, const glm::vec3& initialPosition) override;
private:
};