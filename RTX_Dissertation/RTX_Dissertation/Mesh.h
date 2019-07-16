#pragma once
#include "Renderer.h"

using namespace glm;

class Mesh
{
public:
	Mesh(ID3D12ResourcePtr vbo);

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

	//******************************************//
	//Set the pointer to the BLAS buffer
	//******************************************//
	void SetBLAS(ID3D12ResourcePtr val) { mBLAS = val; };

	auto GetVBO() { return mVBO; };
	auto GetBLAS() { return mBLAS; };
	auto& GetInstances() { return mInstances; };
private:
	//Ptr to the VBO
	ID3D12ResourcePtr mVBO = nullptr;
	//Ptr to the BLAS
	ID3D12ResourcePtr mBLAS = nullptr;

	//List of instances -- TODO: Upgrade to something more flexible in the future such as an instance class.
	std::vector<mat4> mInstances;

};

