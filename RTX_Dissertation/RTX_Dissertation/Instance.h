#pragma once
#include "RendererUtil.h"


class Instance
{
public:
	Instance(glm::mat4 transform, std::vector<std::shared_ptr<HitProgram>> hitPrograms, std::vector<ID3D12ResourcePtr>& resources);
	

	glm::mat4& GetTransform() { return mTransform; };
	std::vector<std::shared_ptr<HitProgram>> GetHitProgram() const { return mHitPrograms; };
	std::vector<ID3D12ResourcePtr> GetResources() const { return mResources; };


	void SetTransform(glm::mat4 val) { mTransform = val; }
	void AddHitGroup(std::shared_ptr<HitProgram> val) { mHitPrograms.push_back(val); };
	void AddResource(ID3D12ResourcePtr val) { mResources.push_back(val); };

private:
	glm::mat4 mTransform;
	std::vector<std::shared_ptr<HitProgram>> mHitPrograms;
	std::vector<ID3D12ResourcePtr> mResources;

};

