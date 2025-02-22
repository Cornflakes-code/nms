#include "TextRendererDynamic.h"

#include <glm/gtc/matrix_transform.hpp>

#include "../Geometry/BoundingBox.h"
#include "../Core/ErrorHandling.h"

#include "../Helpers/MeshDataLight.h"
#include "../Helpers/Shader.h"
#include "../Component/TextComponent.h"

static ShaderData* getDynamicTextData()
{
	ShaderData* shaderData = new ShaderData();
	shaderData->shaderV = "textDynamicBillboard.v.glsl";
	shaderData->shaderF = "text.f.glsl";
	shaderData->shaderG = "";
	shaderData->PVMName = "VP";
	return shaderData;
}

TextRendererDynamic::TextRendererDynamic()
	: TextRenderer(new Shader(getDynamicTextData()))
{
}

void TextRendererDynamic::doSetup(const TextComponent* td, const glm::vec3& initialPosition)
{
	glm::vec3 position = glm::vec3(mBounds.center()) + initialPosition;
	shader()->appendMutator([position](const glm::mat4& proj, const glm::mat4& view,
									const glm::mat4& model, const glm::vec3& cameraPos,
									const Shader* shader)
	{
		glm::vec3 CameraRight_worldspace = { view[0][0], view[1][0], view[2][0] };
		shader->setVector3f("CameraRight_worldspace", CameraRight_worldspace);
		glm::vec3 CameraUp_worldspace = { view[0][1], view[1][1], view[2][1] };
		shader->setVector3f("CameraUp_worldspace", CameraUp_worldspace);
		glm::mat4 newModel = glm::translate(model, position);
		shader->setVector3f("BillboardPos", newModel[3]);
	});
	glm::vec3 sc = td->scale();
	//mBounds.scale(td->scale());
	AABB bounds = mBounds;
	shader()->appendResizer([bounds, sc](const Shader* shader,
			RenderTypes::ScaleByAspectRatioType scaler,
			float aspectRatio)
	{
		shader->setVector2f("BillboardSize", bounds.size() * sc);
	});

}

