#include "GameObject.h"
#include "ResourceManager.h"
#include "ConstantBuffers.h"
using namespace std;
void GameObject::LoadFromJson(nlohmann::basic_json<>::value_type& desc, ID3D12ResourcePtr worldCB)
{
	mName = desc.value("Name", mName);

	//Load mesh
	string meshName = desc.value("Mesh", "");
	auto mesh = ResourceManager::RequestMesh(meshName);

	if(mesh != nullptr)
	{
		auto transformArray = desc["Transform"];
		glm::mat4 transform((float)transformArray.at(0), (float)transformArray.at(1), (float)transformArray.at(2), (float)transformArray.at(3),
			(float)transformArray.at(4), (float)transformArray.at(5), (float)transformArray.at(6), (float)transformArray.at(7),
			(float)transformArray.at(8), (float)transformArray.at(9), (float)transformArray.at(10), (float)transformArray.at(11),
			(float)transformArray.at(12), (float)transformArray.at(13), (float)transformArray.at(14), (float)transformArray.at(15));


		//Load hit groups
		vector<shared_ptr<HitProgram>> hitGroups;
		auto hitGroupArray = desc["HitGroup"];

		for(int i = 0; i < hitGroupArray.size(); i++)
		{
			hitGroups.push_back(ResourceManager::RequestHitProgram(hitGroupArray[i]));
		}

		//Get CBuffers
		vector<ID3D12ResourcePtr> constantBuffers;
		auto bufferArray = desc["Buffers"];
		for (int i = 0; i < bufferArray.size(); i++)
		{
			//Get buffer type. Load correct one and get CB ptr;
			string type = bufferArray[i].value("ConstantBufferType", "");

			if(type == "Material")
			{
				//Load Material
				MaterialBuffer mat{ vec3(bufferArray[i]["MaterialColour"][0], bufferArray[i]["MaterialColour"][1],bufferArray[i]["MaterialColour"][2]), 0,
				vec3(bufferArray[i]["SpecularColour"][0], bufferArray[i]["SpecularColour"][1],bufferArray[i]["SpecularColour"][2]), 0, 
				bufferArray[i].value("SpecularPower", 10) };


				constantBuffers.push_back( RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &mat, sizeof(MaterialBuffer)));

			}
			else if(type == "Metal")
			{
				//Load Metal
				MetalBuffer mb{bufferArray[i].value("Shininess", 0.5f),vec3(0),
					bufferArray[i].value("Scatter", 0.5f),vec3(0) };
				constantBuffers.push_back(RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &mb, sizeof(MetalBuffer)));
			}
			else if(type == "World")
			{
				//Add world cb to buffer array
				constantBuffers.push_back(worldCB);
			}
		}

		Instance inst{ transform, hitGroups, constantBuffers };
		mRendererInstanceIndex = mesh->AddInstance(inst);
	}

	


}

