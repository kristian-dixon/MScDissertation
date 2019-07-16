#include "ResourceManager.h"
#include <Externals/GLM/glm/vec3.hpp>
#include <vector>
#include <Renderer.cpp>

std::map<string, shared_ptr<Mesh>> ResourceManager::mMeshDB;




shared_ptr<Mesh> ResourceManager::RequestMesh(const string& key)
{
	const auto mesh = mMeshDB.find(key);
	if(mesh != mMeshDB.end())
	{
		//Mesh isn't in the database, load mesh and add to DB. 

		//Theoretically this is where a call to a mesh loader would exist. Since we're short on time for now we'll use a factory function to load in either a triangle, plane, cube or a sphere.
		if(key == "TRIANGLE")
		{
			//Create verts
			const vector<glm::vec3> vertices =
			{
				glm::vec3(0,          1,  0),
				glm::vec3(1,  -1, 0),
				glm::vec3(-1, -1, 0),
			};

			auto inst = Renderer::GetInstance();
			if(inst)
			{
				auto mesh = std::make_shared<Mesh>(inst->CreateVertexBuffer(vertices));
				mMeshDB.insert({ key, mesh });
				return mesh;
			}
		}
		else if(key == "QUAD")
		{
			//Create verts
			const vector<glm::vec3> vertices =
			{
				glm::vec3(-1, 1,  0),
				glm::vec3( 1, 1, 0),
				glm::vec3( 1,-1, 0),
				glm::vec3(1,-1, 0),
				glm::vec3(-1,-1, 0),
				glm::vec3(-1, 1,  0),
			};

			auto inst = Renderer::GetInstance();
			if (inst)
			{
				auto mesh = std::make_shared<Mesh>(inst->CreateVertexBuffer(vertices));
				mMeshDB.insert({ key, mesh });
				return mesh;
			}
		}
		else if(key == "CUBE")
		{
			//TODO:LOAD CUBE

		}
		else if(key == "SPHERE")
		{
			//TODO:LOAD SPHERE

		}
		//CYLINDER, CONE and TORUS? (THINK PARAMETIC SHAPES)
		else
		{
			//TODO:Display message saying mesh couldn't be loaded. 

		}
	}
	else
	{
		return mesh->second;
	}
	return shared_ptr<Mesh>();
}
