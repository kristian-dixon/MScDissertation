#include "ResourceManager.h"
#include <Externals/GLM/glm/vec3.hpp>
#include <vector>
#include "Renderer.h"

std::map<string, shared_ptr<Mesh>> ResourceManager::mMeshDB;




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
			const vector<glm::vec3> vertices =
			{
				glm::vec3(0,          1,  0),
				glm::vec3(1,  -1, 0),
				glm::vec3(-1, -1, 0),
			};

			auto inst = Renderer::GetInstance();
			if(inst)
			{
				vector<ID3D12ResourcePtr> vbos = { inst->CreateVertexBuffer(vertices) };
				vector<uint32_t> vertCounts = { static_cast<uint32_t>(vertices.size()) };
				
				auto mesh = std::make_shared<Mesh>( vbos, vertCounts, vector<ID3D12ResourcePtr>(), vector<uint32_t>());
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
				glm::vec3(-1,-1, 0),
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
				vector<ID3D12ResourcePtr> vbos = { inst->CreateVertexBuffer(vertices) };
				vector<uint32_t> vertCounts = { static_cast<uint32_t>(vertices.size()) };

				auto mesh = std::make_shared<Mesh>(vbos, vertCounts, vector<ID3D12ResourcePtr>(), vector<uint32_t>());
				mMeshDB.insert({ key, mesh });
				return mesh;
			}



		}
		else if(key == "SPHERE")
		{
			//TODO:LOAD SPHERE

			float radius = 2.5f;
			uint32 sliceCount = 6;
			uint32 stackCount = 6;
			/*GeometryGenerator::MeshData GeometryGenerator::CreateSphere(float radius, uint32 sliceCount, uint32 stackCount)
			{*/
			std::vector<vec3> verts;
			std::vector<uint32_t> inds;

			//
			// Compute the vertices stating at the top pole and moving down the stacks.
			//

			// Poles: note that there will be texture coordinate distortion as there is
			// not a unique point on the texture map to assign to the pole when mapping
			// a rectangular texture onto a sphere.
			vec3 topVertex = vec3(0.0f, +radius, 0.0f); 
			vec3 bottomVertex = vec3(0.0f, -radius, 0.0f); 

			verts.push_back(topVertex);

			float phiStep = glm::pi<float>() / stackCount;
			float thetaStep = 2.0f * glm::pi<float>() / sliceCount;

			// Compute vertices for each stack ring (do not count the poles as rings).
			for (uint32 i = 1; i <= stackCount - 1; ++i)
			{
				float phi = i * phiStep;

				// Vertices of ring.
				for (uint32 j = 0; j <= sliceCount; ++j)
				{
					float theta = j * thetaStep;
					
					// spherical to cartesian

					float x, y, z = 0;
					x = radius * sinf(phi) * cosf(theta);
					y = radius * cosf(phi);
					z = radius * sinf(phi) * sinf(theta);
					
					vec3 v = vec3(x,y,z);

				
					/*
					// Partial derivative of P with respect to theta
					v.TangentU.x = -radius * sinf(phi)*sinf(theta);
					v.TangentU.y = 0.0f;
					v.TangentU.z = +radius * sinf(phi)*cosf(theta);

					XMVECTOR T = XMLoadFloat3(&v.TangentU);
					XMStoreFloat3(&v.TangentU, XMVector3Normalize(T));

					XMVECTOR p = XMLoadFloat3(&v.Position);
					XMStoreFloat3(&v.Normal, XMVector3Normalize(p));
					
					//v.color.x = theta / XM_2PI;
					//v.color.y = phi / XM_PI;
					*/

					verts.push_back(v);
				}
			}

			verts.push_back(bottomVertex);

			//
			// Compute indices for top stack.  The top stack was written first to the vertex buffer
			// and connects the top pole to the first ring.
			//

			for (uint32_t i = 1; i <= sliceCount; ++i)
			{
				inds.push_back(0);
				inds.push_back(i + 1);
				inds.push_back(i);
			}

			//
			// Compute indices for inner stacks (not connected to poles).
			//

			// Offset the indices to the index of the first vertex in the first ring.
			// This is just skipping the top pole vertex.
			uint32 baseIndex = 1;
			uint32 ringVertexCount = sliceCount + 1;
			for (uint32 i = 0; i < stackCount - 2; ++i)
			{
				for (uint32 j = 0; j < sliceCount; ++j)
				{
					inds.push_back(baseIndex + i * ringVertexCount + j);
					inds.push_back(baseIndex + i * ringVertexCount + j + 1);
					inds.push_back(baseIndex + (i + 1) * ringVertexCount + j);

					inds.push_back(baseIndex + (i + 1) * ringVertexCount + j);
					inds.push_back(baseIndex + i * ringVertexCount + j + 1);
					inds.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
				}
			}

			//
			// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
			// and connects the bottom pole to the bottom ring.
			//

			// South pole vertex was added last.
			uint32 southPoleIndex = (uint32)verts.size() - 1;

			// Offset the indices to the index of the first vertex in the last ring.
			baseIndex = southPoleIndex - ringVertexCount;

			for (uint32 i = 0; i < sliceCount; ++i)
			{
				inds.push_back(southPoleIndex);
				inds.push_back(baseIndex + i);
				inds.push_back(baseIndex + i + 1);
			}

			auto sphereIndCount = inds.size();

			
			
			auto inst = Renderer::GetInstance();
			if (inst)
			{
				vector<ID3D12ResourcePtr> vbos = { inst->CreateVertexBuffer(verts) };
				vector<uint32_t> vertCounts = { static_cast<uint32_t>(verts.size()) };
				vector<ID3D12ResourcePtr> indexBuffers = { inst->CreateIndexBuffer(inds) };
				vector<uint32_t> indexCounts = { static_cast<uint32_t>(inds.size()) };



				auto mesh = std::make_shared<Mesh>(vbos, vertCounts, indexBuffers, indexCounts);
				mMeshDB.insert({ key, mesh });
				return mesh;
			}


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

