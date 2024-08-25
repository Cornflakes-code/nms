#include "NMSRopeScene.h"

#include <Core/Camera.h>
#include <Core/GlobalSettings.h>

#include <Helpers/Shader.h>
#include <Helpers/TextData.h>
#include <Helpers/FreeTypeFontAtlas.h>
#include <Helpers/MeshDataLight.h>
#include <Helpers/Shader.h>
#include <Helpers/ShaderFactory.h>

#include <Renderers/TextRendererDynamic.h>
#include <Renderers/LightRenderer.h>

#include "NMSUserInput.h"
#include "rope_interface_utils.h"
#include "rope_interface_test.h"
#include "rope_quick.h"
#include "ropes.h"
#include "NMSUtils.h"

void NMSRopeScenePhysics::setup()
{
	const AABB& _world = NMSScene::world();
	int fontHeight = 24;
	mNiceSpacing = FreeTypeFontAtlas::FontDetails::pleasingSpacing(
		fontHeight, globals->camera()->aspectRatio());
	mNiceScale = { 5.2f * _world.size().x / globals->physicalWindowSize().x,
						5.2f * _world.size().y / globals->physicalWindowSize().y };

	Rope::initRopes();
	mTextData.set("Ropes", fontHeight, mNiceSpacing * 10.0f, mNiceScale);
	drawRope(_world);
}

void NMSRopeScenePhysics::drawRope(const AABB& _world)
{
	mVectors.clear();
	glm::vec2 ropeZoom = { 500.0f * _world.size().x / globals->physicalWindowSize().x,
					500.0f * _world.size().y / globals->physicalWindowSize().y };

	PolygonBuilder* pb = Rope::drawRope(9239, ropeZoom.x, ropeZoom.y);
	std::vector<PolygonId> polygonIds;
	pb->getAllFloats(mVectors, polygonIds);
	for (const PolygonId& p : polygonIds)
	{
		TextData td;
		td.set(std::to_string(p.id), 10, mNiceSpacing * 10.0f, mNiceScale);
		mPolygonTextData.push_back(std::pair(td, p.pos));
	}
/*
	std::for_each(mVectors.begin(), mVectors.end(),
		[&mMinValuex, &mMaxValuex](const auto& v)
		{
			std::for_each(v.begin(), v.end())
		}
	for (Floats& row : mVectors)
	{
		std::for_each(row.begin(), row.end())
			[&mMinValuex](const auto& elm)
			{

			}
		for (glm::vec3& pt : row)
		{


		}
	}
*/
	Floats temp;
	for (Floats& row : mVectors) 
	{
		std::pair<glm::vec3, glm::vec3> minMax = PolygonBuilder::boundingBox(row);
		temp.push_back(minMax.first);
		temp.push_back(minMax.second);
	}
	mMinMax = PolygonBuilder::boundingBox(temp);
}

void NMSRopeScenePhysics::variableTimeStep(OWUtils::Time::duration OW_UNUSED(dt))
{}

void NMSRopeScenePhysics::fixedTimeStep(std::string& OW_UNUSED(nextSceneName),
	OWUtils::Time::duration OW_UNUSED(dt))
{}

void NMSRopeScenePhysics::interpolateRatio(const ScenePhysicsState* OW_UNUSED(previousState),
	double OW_UNUSED(multPrev),
	const ScenePhysicsState* OW_UNUSED(currentState),
	double OW_UNUSED(multCurr))
{}

void NMSRopeScenePhysics::copy(ScenePhysicsState* source)
{
	*this = *(dynamic_cast<const NMSRopeScenePhysics*>(source));
}

ScenePhysicsState* NMSRopeScenePhysics::clone()
{
	return new NMSRopeScenePhysics(owner());
}

bool NMSRopeScenePhysics::processUserCommands(const UserInput::AnyInput& userInput,
	std::string& nextScene, Camera* OW_UNUSED(camera))
{
	if (userInput.inputType == UserInput::AnyInputType::Pointing)
	{
		// Mouse
	}
	else if ((userInput.inputType == UserInput::AnyInputType::KeyPress) &&
			(userInput.keyInput.action == UserInput::InputAction::Press))
	{
		// Keyboard
		if (userInput.keyInput.userCommand == NMSUserInput::LogicalOperator::OptionsScreen)
		{
			nextScene = Scene::finalSceneName();
			return true;
		}
		else if (userInput.keyInput.userCommand == NMSUserInput::LogicalOperator::Special1)
		{
			const AABB& _world = NMSScene::world();
			drawRope(_world);
			return true;
		}
		else if (userInput.keyInput.userCommand == NMSUserInput::LogicalOperator::OptionsScreen)
		{
			nextScene = Scene::finalSceneName();
			return true;
		}
		else if (userInput.keyInput.userCommand == NMSUserInput::LogicalOperator::Accept)
		{
			nextScene = Scene::previousSceneName();
			return true;
		}
	}
	return false;
}

NMSRopeScene::NMSRopeScene(const Movie* _movie)
	: NMSScene(_movie)
{
	//	const glm::uvec2& screen = globals->physicalWindowSize();
}

void NMSRopeScene::doSetup(ScenePhysicsState* state)
{
	NMSRopeScenePhysics* sps
		= dynamic_cast<NMSRopeScenePhysics*>(state);
	mText = new TextRendererDynamic();
	mText->setup(&(sps->mTextData), glm::vec3(0));

	ModelData md;
	mAxis.setup(&md);

	ShaderFactory shaders;
	Shader* lineShader = new Shader();
	lineShader->loadShaders(shaders.boilerPlateVertexShader(),
		shaders.boilerPlateFragmentShader(),
		shaders.boilerPlateGeometryShader());

	std::vector<glm::vec3> coords;
	for (auto& val : sps->mVectors)
	{
		MeshDataLight lineData;
		lineData.colour(OWUtils::colour(OWUtils::SolidColours::BRIGHT_GREEN), "colour");
		lineData.vertices(val, GL_LINES);
		LightRenderer* lines = new LightRenderer(lineShader, "pvm");
		lines->setup(&lineData);
		md.renderers.push_back(lines);
	}
	for (const std::pair<TextData, glm::vec3>& td : sps->mPolygonTextData)
	{
		TextRendererDynamic* t = new TextRendererDynamic();
		t->setup(&(td.first), td.second);
		mPolyLabels.push_back(t);
	}
	mCircles.setup(&md);
}

void NMSRopeScene::render(const ScenePhysicsState* OW_UNUSED(state),
	const glm::mat4& proj, const glm::mat4& view)
{
	glm::mat4 model(1.0f);
	mText->render(proj, view, model);
	for (auto& t : mPolyLabels)
	{
		t->render(proj, view, model);
	}
	mAxis.render(proj, view, model);
	mCircles.render(proj, view, model);
}

void NMSRopeScene::activate(const std::string& OW_UNUSED(previousScene),
	ScenePhysicsState* state,
	Camera* camera, unsigned int callCount)
{
	NMSRopeScenePhysics* sp = dynamic_cast<NMSRopeScenePhysics*>(state);
	if (!callCount)
	{
		glm::vec3 extent(sp->mMinMax.second.x - sp->mMinMax.first.x,
			sp->mMinMax.second.y - sp->mMinMax.first.y,
			sp->mMinMax.second.z - sp->mMinMax.first.z);

		glm::vec3 center((sp->mMinMax.second.x + sp->mMinMax.first.x) / 2.0f,
			(sp->mMinMax.second.y + sp->mMinMax.first.y) / 2.0f,
			(sp->mMinMax.second.z + sp->mMinMax.first.z) / 2.0f);
		center.z = 200;
		camera->position(center);
		center.z = 0;
		camera->lookAt(center);
		//camera->FOV(glm::radians(45.0f));
	}
}

void NMSRopeScene::deActivate(const Camera* OW_UNUSED(camera),
	ScenePhysicsState* OW_UNUSED(state))
{
}
