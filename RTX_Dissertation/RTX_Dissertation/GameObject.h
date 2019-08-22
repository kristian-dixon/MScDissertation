#pragma once

#include <nlohmann/json.hpp>
#include "ResourceManager.h"


class GameObject
{
public:

	void LoadFromJson(nlohmann::basic_json<>::value_type& desc, ID3D12ResourcePtr worldCB);
private:
	std::string mName = "";
	shared_ptr<Mesh> mMesh = nullptr;
	int mRendererInstanceIndex = -1;
};

