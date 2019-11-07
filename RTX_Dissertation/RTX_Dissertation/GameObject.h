#pragma once

#include <nlohmann/json.hpp>
#include "ResourceManager.h"
#include "GameComponent.h"


class GameObject
{
public:

	void LoadFromJson(nlohmann::basic_json<>::value_type& desc, ID3D12ResourcePtr worldCB);

	auto GetComponentMask() const { return mComponentMask; };

	auto& GetOriginalTransform() { return mOriginalTransform; };

	glm::mat4 GetCurrentTransform() const { return mMesh->GetInstance(mRendererInstanceIndex).GetTransform(); };

	void SetTransform(glm::mat4& val) { mMesh->GetInstance(mRendererInstanceIndex).SetTransform(val); };

	shared_ptr<GameComponent> GetComponent(EComponentType componentType) { for (int i = 0; i < mComponents.size(); i++) if (mComponents[i]->GetComponentType() == componentType) return mComponents[i]; return nullptr; }

private:
	std::string mName = "";
	shared_ptr<Mesh> mMesh = nullptr;
	vector<shared_ptr<GameComponent>> mComponents;

	glm::mat4 mOriginalTransform; 

	int mRendererInstanceIndex = -1;
	int mComponentMask = 0;

	
};

