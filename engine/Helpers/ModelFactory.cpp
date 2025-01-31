#include "ModelFactory.h"
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb/stb_image.h"

#include "../Core/GlobalSettings.h"
#include "../Core/LogStream.h"
#include "../Core/ResourcePathFactory.h"

#include "ModelData.h"
#include "MeshDataHeavy.h"
#include "Texture.h"

ModelFactory::ModelFactory()
{}

ModelData processNode(aiNode *node, const aiScene *scene);

ModelData ModelFactory::create(const std::string& modelFileName, bool cache)
{
	std::filesystem::path modelPath =
		ResourcePathFactory().appendPath(modelFileName,
			ResourcePathFactory::ResourceType::Model);

	Assimp::Importer importer;
	const aiScene *aiscene = importer.ReadFile(modelPath.string(),
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded); // | aiProcess_FlipUVs);

	if (!aiscene || aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiscene->mRootNode)
	{
		LogStream(LogStreamLevel::Info)
			<< "Mesh load failed for model ["
			<< modelPath << "] ASSIMP error [" << importer.GetErrorString() << "]";
		return ModelData();
	}
	ModelData root = processNode(aiscene->mRootNode, aiscene);
	return root;
}

ModelData processNode(aiNode *node, const aiScene *scene)
{
	ModelData retval;

	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		MeshDataHeavy* m = new MeshDataHeavy();
		m->create(scene->mMeshes[node->mMeshes[i]], scene);
		retval.meshes.push_back(m);
	}

	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ModelData child = processNode(node->mChildren[i], scene);
		retval.children.push_back(child);
	}
	return retval;
}
