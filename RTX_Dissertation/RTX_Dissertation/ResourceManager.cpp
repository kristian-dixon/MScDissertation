#include "ResourceManager.h"
#include <Externals/GLM/glm/vec3.hpp>
#include <vector>
#include "Renderer.h"

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
				vector<ID3D12Resource*> vbos = { inst->CreateVertexBuffer(vertices) };
				vector<uint32_t> vertCounts = { static_cast<uint32_t>(vertices.size()) };
				
				auto mesh = std::make_shared<Mesh>( vbos, vertCounts );
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
				vector<ID3D12Resource*> vbos = { inst->CreateVertexBuffer(vertices) };
				vector<uint32_t> vertCounts = { static_cast<uint32_t>(vertices.size()) };

				auto mesh = std::make_shared<Mesh>(vbos, vertCounts);
				mMeshDB.insert({ key, mesh });
				return mesh;
			}
		}
		else if(key == "CUBE")
		{
			//TODO:LOAD CUBE

			//Create verts
			const vector<glm::vec3> vertices =
			{
				glm::vec3(-1.0f,-1.0f,-1.0f), // triangle 1 : begin
				glm::vec3(-1.0f,-1.0f, 1.0f),
				glm::vec3(-1.0f, 1.0f, 1.0f), // triangle 1 : end
				glm::vec3(1.0f, 1.0f,-1.0f), // triangle 2 : begin
				glm::vec3(-1.0f,-1.0f,-1.0f),
				glm::vec3(-1.0f, 1.0f,-1.0f), // triangle 2 : end
				glm::vec3(1.0f,-1.0f, 1.0f),
				glm::vec3(-1.0f,-1.0f,-1.0f),
				glm::vec3(1.0f,-1.0f,-1.0f),
				glm::vec3(1.0f, 1.0f,-1.0f),
				glm::vec3(1.0f,-1.0f,-1.0f),
				glm::vec3(-1.0f,-1.0f,-1.0f),
				glm::vec3(-1.0f,-1.0f,-1.0f),
				glm::vec3(-1.0f, 1.0f, 1.0f),
				glm::vec3(-1.0f, 1.0f,-1.0f),
				glm::vec3(1.0f,-1.0f, 1.0f),
				glm::vec3(-1.0f,-1.0f, 1.0f),
				glm::vec3(-1.0f,-1.0f,-1.0f),
				glm::vec3(-1.0f, 1.0f, 1.0f),
				glm::vec3(-1.0f,-1.0f, 1.0f),
				glm::vec3(1.0f,-1.0f, 1.0f),
				glm::vec3(1.0f, 1.0f, 1.0f),
				glm::vec3(1.0f,-1.0f,-1.0f),
				glm::vec3(1.0f, 1.0f,-1.0f),
				glm::vec3(1.0f,-1.0f,-1.0f),
				glm::vec3(1.0f, 1.0f, 1.0f),
				glm::vec3(1.0f,-1.0f, 1.0f),
				glm::vec3(1.0f, 1.0f, 1.0f),
				glm::vec3(1.0f, 1.0f,-1.0f),
				glm::vec3(-1.0f, 1.0f,-1.0f),
				glm::vec3(1.0f, 1.0f, 1.0f),
				glm::vec3(-1.0f, 1.0f,-1.0f),
				glm::vec3(-1.0f, 1.0f, 1.0f),
				glm::vec3(1.0f, 1.0f, 1.0f),
				glm::vec3(-1.0f, 1.0f, 1.0f),
				glm::vec3(1.0f,-1.0f, 1.0f)
			};

			auto inst = Renderer::GetInstance();
			if (inst)
			{
				vector<ID3D12Resource*> vbos = { inst->CreateVertexBuffer(vertices) };
				vector<uint32_t> vertCounts = { static_cast<uint32_t>(vertices.size()) };

				auto mesh = std::make_shared<Mesh>(vbos, vertCounts);
				mMeshDB.insert({ key, mesh });
				return mesh;
			}



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

