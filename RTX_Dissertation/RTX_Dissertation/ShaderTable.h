#pragma once

#include "RendererUtil.h"

class ShaderTable
{
public:
	//Figure out the shader table entry size
	//Get largest RTSPO use that

	//Figure out shader table size
	void AddMissProgram(MissProgram val) { missPrograms.push_back(val); };

	//Raygen shader
	//List of miss shaders
	//List of instances

	void BuildShaderTable(HWND windowHandle, ID3D12Device5Ptr device, ID3D12StateObjectPtr mpPipelineState, AccelerationStructureBuffers& tlas, ID3D12DescriptorHeapPtr mpSrvUavHeap);

	ID3D12ResourcePtr GetShaderTable() { return mpShaderTable; };

	uint32_t GetMissShaderCount() { return missPrograms.size(); };
	uint32_t GetShaderTableEntryCount() { return mShaderTableEntryCount; };
	uint32_t GetShaderTableEntrySize() { return mShaderTableEntrySize; };


private:
	uint32_t mShaderTableEntrySize = 0;
	uint32_t mShaderTableEntryCount = 0;

	std::vector<MissProgram> missPrograms;



	ID3D12ResourcePtr mpShaderTable;

};

