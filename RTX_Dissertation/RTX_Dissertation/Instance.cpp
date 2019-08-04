#include "Instance.h"

Instance::Instance(glm::mat4 transform, std::shared_ptr<HitProgram> hitProgram, std::vector<ID3D12ResourcePtr>& resources) : 
	mTransform(transform), mHitProgram(hitProgram), mResources(resources)
{

}
