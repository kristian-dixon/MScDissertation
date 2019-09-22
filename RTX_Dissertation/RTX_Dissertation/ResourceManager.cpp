#include "ResourceManager.h"
#include <Externals/GLM/glm/vec3.hpp>
#include <vector>
#include "Renderer.h"
#include "Vertex.h"

#include <d3d12.h>
//#include <SimpleMath.h>
#include "ObjLoader.h"
#include "RendererUtil.h"

std::map<string, shared_ptr<Mesh>> ResourceManager::mMeshDB;
std::map<string, shared_ptr<HitProgram>> ResourceManager::mHitProgramDB;
std::map<string, shared_ptr<MissProgram>> ResourceManager::mMissProgramDB;



shared_ptr<Mesh> ResourceManager::RequestMesh(const string& key)
{

	const auto mesh = mMeshDB.find(key);
	if(mesh == mMeshDB.end())
	{
		//Mesh isn't in DB

		//Theoretically this is where a call to a mesh loader would exist. Since we're short on time for now we'll use a factory function to load in either a triangle, plane, cube or a sphere.
		auto mesh = ObjLoader::LoadOBJMesh("Data/Models/" + key + ".obj");
		if (mesh != nullptr)
		{
			mMeshDB.insert({ key, mesh });
		}
		else
		{
			//RendererUtil::DisplayMessage(Renderer::GetInstance()->GetWindowHandle(), key + " mesh could not be found! ", "Warning");
			//Create dummy intersection shader for now - not great but it'll do for now

			D3D12_RAYTRACING_AABB aabb { -10,-10,-10, 10, 10, 10 };
			auto buffer = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &aabb, sizeof(D3D12_RAYTRACING_AABB));

			auto aabbMesh = std::make_shared<Mesh>(buffer);
			mMeshDB.insert({key, aabbMesh});
			return aabbMesh;
		}
		return mesh;
	}
	else
	{
		return mesh->second;
	}
}

shared_ptr<Mesh> ResourceManager::AddNewMesh(const string& key, const vector<glm::vec3> verts)
{
	//TODO:: Check if mesh entry already exists
	//TODO:: Create buffer on GPU
	//TODO:: Add to DB
	return shared_ptr<Mesh>();
}

shared_ptr<Mesh> ResourceManager::AddNewMesh(const string& key, ID3D12ResourcePtr vbo)
{
	//TODO:: Check if already exists.
	//TODO:: ADD TO DB
	return shared_ptr<Mesh>();
}

shared_ptr<HitProgram> ResourceManager::RequestHitProgram(const string& key)
{
	const auto hitProgram = mHitProgramDB.find(key);

	if(hitProgram != mHitProgramDB.end())
	{
		return hitProgram->second;
	}
	else
	{
		RendererUtil::DisplayMessage(Renderer::GetInstance()->GetWindowHandle(), key + " hitgroup could not be found! ", "Warning");
		//TODO:: Set error shader?
	}
	return nullptr;
}

void ResourceManager::AddHitProgram(const string& key, shared_ptr<HitProgram> hitProgram)
{
	 mHitProgramDB.insert({ key, hitProgram }); 
}

void ResourceManager::AddMissProgram(const string& key, shared_ptr<MissProgram> missProgram)
{
	mMissProgramDB.insert({ key, missProgram });
}
