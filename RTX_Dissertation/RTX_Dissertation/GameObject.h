#pragma once

#include <nlohmann/json.hpp>
#include "ResourceManager.h"
#include "GameComponent.h"


class GameObject
{
public:

	void LoadFromJson(nlohmann::basic_json<>::value_type& desc, ID3D12ResourcePtr worldCB);

	auto GetComponentMask() const { return mComponentMask; };

private:
	std::string mName = "";
	shared_ptr<Mesh> mMesh = nullptr;
	vector<shared_ptr<GameComponent>> mComponents;

	int mRendererInstanceIndex = -1;
	int mComponentMask = 0;

	
};

