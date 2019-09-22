#pragma once
#include "Renderer.h"
#include "Instance.h"
#include <memory>
using namespace glm;

class Mesh
{
public:
	Mesh(std::vector<ID3D12ResourcePtr>& vbo, std::vector<uint32_t>& vertCounts, std::vector<ID3D12ResourcePtr> &indexPtrs, std::vector<uint32_t>& indexCounts);
	Mesh(ID3D12ResourcePtr aabb);
	

	//**********************************************//
	//Adds an instance to be uploaded into the TLAS
	//Returns: index of instance entry
	//**********************************************//
	size_t AddInstance(Instance& val);

	//******************************************//
	//Removes an instance from the TLAS
	//id: index of instance to be removed.
	//******************************************//
	void RemoveInstance(size_t id);

	void UpdateInstance(size_t id, Instance& val) { mInstances[id] = val; }
	int GetInstanceCount() const { return mInstanceCount; };


	//******************************************//
	//Set the pointer to the BLAS buffer
	//******************************************//
	void SetBLAS(AccelerationStructureBuffers val) { mBLAS = val; };

	std::vector<ID3D12ResourcePtr>& GetVBOs() { return mVBOs; };
	AccelerationStructureBuffers GetBLAS() const { return mBLAS; };
	std::vector<Instance>& GetInstances() { return mInstances; };
	const std::vector<uint32_t>& GetVertexCounts() const { return mVertexCounts; };
	
	std::vector<ID3D12ResourcePtr>& GetIndices() { return mIndices; };
	const std::vector<uint32_t>& GetIndexCounts() const { return mIndicesCounts; };

	ID3D12ResourcePtr GetAABB() { return aabbBuffer; }


private:
	//Ptr to the VBO
	std::vector<ID3D12ResourcePtr> mVBOs;
	std::vector<uint32_t>mVertexCounts;

	//Index data
	std::vector<ID3D12ResourcePtr> mIndices;
	std::vector<uint32_t> mIndicesCounts;

	//AABB data
	ID3D12ResourcePtr aabbBuffer = nullptr;

	//Ptr to the BLAS
	AccelerationStructureBuffers mBLAS;

	ID3D12DescriptorHeapPtr mDescriptorHeap = nullptr;

	//List of instances -- TODO: Upgrade to something more flexible in the future such as an instance class.
	std::vector<Instance> mInstances;

	int mInstanceCount = 0;

};

