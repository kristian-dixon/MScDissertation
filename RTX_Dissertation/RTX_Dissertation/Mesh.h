#pragma once
#include "Renderer.h"
#include <memory>
using namespace glm;

class Mesh
{
public:
	Mesh(std::vector<ID3D12ResourcePtr>& vbo, std::vector<uint32_t>& vertCounts);

	//hmm pass by ref?
	//**********************************************//
	//Adds an instance to be uploaded into the TLAS
	//Returns: index of instance entry
	//**********************************************//
	size_t AddInstance(mat4& transform);

	//******************************************//
	//Removes an instance from the TLAS
	//id: index of instance to be removed.
	//******************************************//
	void RemoveInstance(size_t id);

	void UpdateInstance(size_t id, mat4& transform) { mInstances[id] = transform; };
	int GetInstanceCount() const { return mInstanceCount; };



	//******************************************//
	//Set the pointer to the BLAS buffer
	//******************************************//
	void SetBLAS(ID3D12ResourcePtr val) { mBLAS = val; };

	std::vector<ID3D12ResourcePtr>& GetVBOs() { return mVBOs; };
	ID3D12ResourcePtr GetBLAS() const { return mBLAS; };
	std::vector<mat4>& GetInstances() { return mInstances; };
	std::vector<uint32_t> GetVertexCounts() const { return mVertexCounts; };
private:
	//Ptr to the VBO
	std::vector<ID3D12ResourcePtr> mVBOs;
	std::vector<uint32_t>mVertexCounts;
	
	//Ptr to the BLAS
	ID3D12ResourcePtr mBLAS = nullptr;

	//List of instances -- TODO: Upgrade to something more flexible in the future such as an instance class.
	std::vector<mat4> mInstances;

	int mInstanceCount = 0;

};

