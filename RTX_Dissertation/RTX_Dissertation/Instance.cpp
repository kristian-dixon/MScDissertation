#include "Instance.h"

Instance::Instance(glm::mat4 transform, std::vector<std::shared_ptr<HitProgram>> hitPrograms, std::vector<ID3D12ResourcePtr>& resources) :
	mTransform(transform), mHitPrograms(hitPrograms), mResources(resources)
{

}
