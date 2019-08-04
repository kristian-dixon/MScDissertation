#pragma once
#include "RendererUtil.h"


class Instance
{
public:
	Instance(glm::mat4 transform, std::shared_ptr<HitProgram> hitProgram, std::vector<ID3D12ResourcePtr>& resources);
	

	glm::mat4& GetTransform() { return mTransform; };
	std::shared_ptr<HitProgram> GetHitProgram() const { return mHitProgram; };
	std::vector<ID3D12ResourcePtr> GetResources() const { return mResources; };


	void SetTransform(glm::mat4 val) { mTransform = val; }
	void SetHitGroup(std::shared_ptr<HitProgram> val) { mHitProgram = val; };
	void AddResource(ID3D12ResourcePtr val) { mResources.push_back(val); };

private:
	glm::mat4 mTransform;
	std::shared_ptr<HitProgram> mHitProgram;
	std::vector<ID3D12ResourcePtr> mResources;

};

