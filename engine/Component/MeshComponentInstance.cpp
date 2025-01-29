#include "MeshComponentInstance.h"

void MeshComponentInstance::init()
{
	MeshComponentInstanceData* d = data();
	InstanceRenderer* r = new InstanceRenderer(new Shader(&d->shaderData));
	r->setup(&d->meshData);
	//d->boundingBox = d->meshData.bounds();
	addRenderer(r);
}
