#include "Instance.h"
#include "Renderer.h"

Instance::Instance(glm::mat4 transform, std::vector<std::shared_ptr<HitProgram>> hitPrograms, std::vector<ID3D12ResourcePtr>& resources) :
	mHitPrograms(hitPrograms), mResources(resources)
{
	SetTransform(transform);
}

void Instance::SetTransform(glm::mat4 val)
{
	mTransform = val;
	mTransformCB = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &mTransform, sizeof(mat4));
}
