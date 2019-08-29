#include "ResourceManager.h"
#include <Externals/GLM/glm/vec3.hpp>
#include <vector>
#include "Renderer.h"
#include "Vertex.h"

#include <d3d12.h>
#include <SimpleMath.h>
#include "ObjLoader.h"

std::map<string, shared_ptr<Mesh>> ResourceManager::mMeshDB;
std::map<string, shared_ptr<HitProgram>> ResourceManager::mHitProgramDB;


shared_ptr<Mesh> ResourceManager::RequestMesh(const string& key)
{
	
	const auto mesh = mMeshDB.find(key);
	if(mesh == mMeshDB.end())
	{
		//Mesh isn't in the database, load mesh and add to DB. 

		//Theoretically this is where a call to a mesh loader would exist. Since we're short on time for now we'll use a factory function to load in either a triangle, plane, cube or a sphere.
		if(key == "TRIANGLE")
		{
			//Create verts
			const vector<Vertex> vertices =
			{
				{glm::vec4(0,   1, 0,0),  glm::vec3(0,0,-1) },
				{glm::vec4(1,  -1, 0,0),  glm::vec3(0,0,-1) },
				{glm::vec4(-1,  -1, 0,0),  glm::vec3(0,0,-1) }
			};

			const vector<uint32_t> indices =
			{
				0,1,2
			};

			auto inst = Renderer::GetInstance();
			if(inst)
			{
				vector<ID3D12ResourcePtr> vbos = { inst->CreateVertexBuffer(vertices) };
				vector<uint32_t> vertCounts = { static_cast<uint32_t>(vertices.size()) };
				vector<ID3D12ResourcePtr> indexBuffers = { inst->CreateIndexBuffer(indices) };
				vector<uint32_t> indexCounts = { static_cast<uint32_t>(indices.size()) };

				auto mesh = std::make_shared<Mesh>(vbos, vertCounts, indexBuffers, indexCounts);
				mMeshDB.insert({ key, mesh });
				return mesh;
			}
		}
		else if(key == "QUAD")
		{
			//Create verts
			const vector<Vertex> vertices =
			{
				{glm::vec4(-1, 1, 0,0), glm::vec3(0, 0, -1)},
				{glm::vec4(1, 1, 0,0),  glm::vec3(0, 0, -1)},
				{glm::vec4(1, -1, 0,0),  glm::vec3(0, 0,-1)},
				{glm::vec4(-1,-1, 0,0),  glm::vec3(0, 0, -1)},
			};


			const vector<uint32_t> indices =
			{
				0,1,2,
				2,3,0
			};


			auto inst = Renderer::GetInstance();
			if (inst)
			{
				vector<ID3D12ResourcePtr> vbos = { inst->CreateVertexBuffer(vertices) };
				vector<uint32_t> vertCounts = { static_cast<uint32_t>(vertices.size()) };
				vector<ID3D12ResourcePtr> indexBuffers = { inst->CreateIndexBuffer(indices) };
				vector<uint32_t> indexCounts = { static_cast<uint32_t>(indices.size()) };
				


				auto mesh = std::make_shared<Mesh>(vbos, vertCounts, indexBuffers, indexCounts);
				mMeshDB.insert({ key, mesh });
				return mesh;
			}
		}
		else if(key == "CUBE")
		{
			//Create verts
			const vector<Vertex> vertices =
			{
				{glm::vec4(-0.5f,-0.5f,-0.5f,0), glm::vec3(-1, 0, 0) , 1}, // triangle 1 : begin
				{glm::vec4(-0.5f,-0.5f, 0.5f, 0), glm::vec3(-1, 0, 0),1},
				{glm::vec4(-0.5f, 0.5f, 0.5f,0), glm::vec3(-1, 0, 0), 1}, // triangle 1 : end
				{glm::vec4(0.5f, 0.5f,-0.5f,0), glm::vec3(0, 0, -1) }, // triangle 2 : begin
				{glm::vec4(-0.5f,-0.5f,-0.5f,0), glm::vec3(0, 0, -1)},
				{glm::vec4(-0.5f, 0.5f,-0.5f,0),  glm::vec3(0, 0, -1)}, // triangle 2 : end
				{glm::vec4(0.5f,-0.5f, 0.5f,0),  glm::vec3(0, -1, 0)},
				{glm::vec4(-0.5f,-0.5f,-0.5f,0),  glm::vec3(0, -1, 0)},
				{glm::vec4(0.5f,-0.5f,-0.5f,0),  glm::vec3(0, -1, 0)},
				{glm::vec4(0.5f, 0.5f,-0.5f,0),  glm::vec3(0, 0, -1)},
				{glm::vec4(0.5f,-0.5f,-0.5f,0),  glm::vec3(0, 0, -1)},
				{glm::vec4(-0.5f,-0.5f,-0.5f,0),  glm::vec3(0, 0, -1)},
				{glm::vec4(-0.5f,-0.5f,-0.5f,0),  glm::vec3(-1, 0, 0)},
				{glm::vec4(-0.5f, 0.5f, 0.5f,0),  glm::vec3(-1, 0, 0)},
				{glm::vec4(-0.5f, 0.5f,-0.5f,0),  glm::vec3(-1, 0, 0)},
				{glm::vec4(0.5f,-0.5f, 0.5f,0),  glm::vec3(0, -1, 0)},
				{glm::vec4(-0.5f,-0.5f, 0.5f,0),  glm::vec3(0, -1, 0)},
				{glm::vec4(-0.5f,-0.5f,-0.5f,0),  glm::vec3(0, -1, 0)},
				{glm::vec4(-0.5f, 0.5f, 0.5f,0),  glm::vec3(0, 0, 1)},
				{glm::vec4(-0.5f,-0.5f, 0.5f,0),  glm::vec3(0, 0, 1)},
				{glm::vec4(0.5f,-0.5f, 0.5f,0),  glm::vec3(0, 0, 1)},
				{glm::vec4(0.5f, 0.5f, 0.5f,0),  glm::vec3(1, 0, 0)},
				{glm::vec4(0.5f,-0.5f,-0.5f,0),  glm::vec3(1, 0, 0)},
				{glm::vec4(0.5f, 0.5f,-0.5f,0),  glm::vec3(1, 0, 0)},
				{glm::vec4(0.5f,-0.5f,-0.5f,0),  glm::vec3(1, 0, 0)},
				{glm::vec4(0.5f, 0.5f, 0.5f,0),  glm::vec3(1, 0, 0)},
				{glm::vec4(0.5f,-0.5f, 0.5f,0),  glm::vec3(1, 0, 0)},
				{glm::vec4(0.5f, 0.5f, 0.5f,0),  glm::vec3(0, 1, 0)},
				{glm::vec4(0.5f, 0.5f,-0.5f,0),  glm::vec3(0, 1, 0)},
				{glm::vec4(-0.5f, 0.5f,-0.5f,0),  glm::vec3(0, 1, 0)},
				{glm::vec4(0.5f, 0.5f, 0.5f,0),  glm::vec3(0, 1, 0)},
				{glm::vec4(-0.5f, 0.5f,-0.5f,0),  glm::vec3(0, 1, 0)},
				{glm::vec4(-0.5f, 0.5f, 0.5f,0),  glm::vec3(0, 1, 0)},
				{glm::vec4(0.5f, 0.5f, 0.5f,0),  glm::vec3(0, 0, 1)},
				{glm::vec4(-0.5f, 0.5f, 0.5f,0),  glm::vec3(0, 0, 1)},
				{glm::vec4(0.5f,-0.5f, 0.5f,0), glm::vec3(0, 0, 1)}
			};

			//Bad but from now on indexed buffers are more important
			vector<uint32_t> indices;
			indices.resize(vertices.size());
			for(int i = 0; i < vertices.size(); i++)
			{
				indices[i] = i;
			}


			auto inst = Renderer::GetInstance();
			if (inst)
			{
				vector<ID3D12ResourcePtr> vbos = { inst->CreateVertexBuffer(vertices) };
				vector<uint32_t> vertCounts = { static_cast<uint32_t>(vertices.size()) };

				vector<ID3D12ResourcePtr> indexBuffers = { inst->CreateIndexBuffer(indices) };
				vector<uint32_t> indexCounts = { static_cast<uint32_t>(indices.size()) };

				auto mesh = std::make_shared<Mesh>(vbos, vertCounts, indexBuffers, indexCounts);
				mMeshDB.insert({ key, mesh });
				return mesh;
			}



		}
		else if(key == "SPHERE")
		{
			auto mesh = ObjLoader::LoadOBJMesh("ObjTestFile.obj");
			if(mesh != nullptr)
			{
				mMeshDB.insert({ key, mesh });
			}
			return mesh;
		}
		//CYLINDER, CONE and TORUS? (THINK PARAMETIC SHAPES)
		else
		{
			//Attempt to load from disk
			/*auto test = DirectX::Model::CreateFromSDKMESH(L"modelTest.skdmesh", Renderer::GetInstance()->GetDevice());

			//TODO:: Update for multiple
			vector<ID3D12ResourcePtr> vbos = { test->meshes[0]->opaqueMeshParts[0]->vertexBuffer.Resource() };
			vector<uint32_t> vertCounts = { test->meshes[0]->opaqueMeshParts[0]->vertexBufferSize };
			vector<ID3D12ResourcePtr> indexBuffers = { test->meshes[0]->opaqueMeshParts[0]->indexBuffer.Resource() };
			vector<uint32_t> indexCounts = { test->meshes[0]->opaqueMeshParts[0]->indexBufferSize };


			auto mesh = std::make_shared<Mesh>(vbos, vertCounts, indexBuffers, indexCounts);
			mMeshDB.insert({ key, mesh });
			return mesh;*/
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

shared_ptr<HitProgram> ResourceManager::RequestHitProgram(const string& key)
{
	const auto hitProgram = mHitProgramDB.find(key);

	if(hitProgram != mHitProgramDB.end())
	{
		return hitProgram->second;
	}
	return nullptr;
}

void ResourceManager::AddHitProgram(const string& key, shared_ptr<HitProgram> hitProgram)
{
	 mHitProgramDB.insert({ key, hitProgram }); 
}

