#include "GameObject.h"
#include "ResourceManager.h"
#include "ConstantBuffers.h"
using namespace std;
void GameObject::LoadFromJson(nlohmann::basic_json<>::value_type& desc, ID3D12ResourcePtr worldCB)
{
	mName = desc.value("Name", mName);

	//Load mesh
	string meshName = desc.value("Mesh", "");

	if(meshName == "")
	{
		//TODO:: Load intersection aabb
		meshName = "IntersectionAABB";
	}

	auto mesh = ResourceManager::RequestMesh(meshName);

	if(mesh != nullptr)
	{
		auto transformArray = desc["Transform"];
		glm::mat4 transform((float)transformArray[0], (float)transformArray.at(1), (float)transformArray.at(2), (float)transformArray.at(3),
			(float)transformArray.at(4), (float)transformArray.at(5), (float)transformArray.at(6), (float)transformArray.at(7),
			(float)transformArray.at(8), (float)transformArray.at(9), (float)transformArray.at(10), (float)transformArray[11],
			(float)transformArray.at(12), (float)transformArray.at(13), (float)transformArray.at(14), (float)transformArray.at(15));
		transform = glm::transpose(transform);

		//Load hit groups
		vector<shared_ptr<HitProgram>> hitGroups;
		auto hitGroupArray = desc["HitGroups"];

		for(int i = 0; i < hitGroupArray.size(); i++)
		{
			auto hitGroup = ResourceManager::RequestHitProgram(hitGroupArray[i]);

			if (hitGroup == nullptr) { return; }
			hitGroups.push_back(hitGroup);

			
		}

		//Get CBuffers
		vector<ID3D12ResourcePtr> constantBuffers;
		
		//Get Metal buffer
		auto metalDesc = desc["Metal"];
		if(metalDesc.value("ConstantBufferType", "") == "Metal")
		{
			//Load Metal
			MetalBuffer mb{ metalDesc.value("Shininess", 0.5f),vec3(0),
				metalDesc.value("Scatter", 0.5f),vec3(0) };
			constantBuffers.push_back(RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &mb, sizeof(MetalBuffer)));
		}

		//Get Material Buffer
		auto materialDesc = desc["Material"];
		if (materialDesc.value("ConstantBufferType", "") == "Material")
		{
			//Load Material
			MaterialBuffer mat{ vec3(materialDesc["MaterialColour"][0], materialDesc["MaterialColour"][1],materialDesc["MaterialColour"][2]), 0,
			vec3(materialDesc["SpecularColour"][0], materialDesc["SpecularColour"][1],materialDesc["SpecularColour"][2]), 0,
			materialDesc.value("SpecularPower", 10) };

			constantBuffers.push_back(RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &mat, sizeof(MaterialBuffer)));
		}


		//Get Material Buffer
		auto worldDesc = desc["WorldData"];
		if (worldDesc.value("ConstantBufferType", "") == "World")
		{
			constantBuffers.push_back(worldCB);
		}

		Instance inst{ transform, hitGroups, constantBuffers };
		mRendererInstanceIndex = static_cast<int>(mesh->AddInstance(inst));
	}

	


}

