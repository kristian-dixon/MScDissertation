#include "ShaderTable.h"
#include "ResourceManager.h"

void ShaderTable::BuildShaderTable(HWND windowHandle, ID3D12Device5Ptr device, ID3D12StateObjectPtr mpPipelineState, AccelerationStructureBuffers& tlas, ID3D12DescriptorHeapPtr mpSrvUavHeap)
{
	const WCHAR* kRayGenShader = L"rayGen";

	// TODO:: Make this nicer
	mShaderTableEntrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	mShaderTableEntrySize += 32; // The ray-gen's descriptor table
	mShaderTableEntrySize = align_to(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, mShaderTableEntrySize);

	//Raygen 
	int entryCount = 1;

	entryCount += missPrograms.size();

	auto meshDB = ResourceManager::GetMeshDB();
	for (auto& mesh : meshDB)
	{
		for (auto& instances : mesh.second->GetInstances())
		{
			entryCount+= instances.GetHitPrograms().size();
		}
	}

	mShaderTableEntryCount = entryCount;


	uint32_t shaderTableSize = mShaderTableEntrySize * entryCount;

	// For simplicity, we create the shader-table on the upload heap. You can also create it on the default heap
	mpShaderTable = RendererUtil::CreateBuffer(windowHandle, device, shaderTableSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, RendererUtil::kUploadHeapProps);

	// Map the buffer
	uint8_t* pData;
	RendererUtil::D3DCall(windowHandle, mpShaderTable->Map(0, nullptr, (void**)& pData));

	MAKE_SMART_COM_PTR(ID3D12StateObjectProperties);
	ID3D12StateObjectPropertiesPtr pRtsoProps;
	mpPipelineState->QueryInterface(IID_PPV_ARGS(&pRtsoProps));

	// Entry 0 - ray-gen program ID and descriptor data
	memcpy(pData, pRtsoProps->GetShaderIdentifier(kRayGenShader), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
	uint64_t heapStart = mpSrvUavHeap->GetGPUDescriptorHandleForHeapStart().ptr;
	*(uint64_t*)(pData + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES) = heapStart;
	// This is where we need to set the descriptor data for the ray-gen shader. We'll get to it in the next tutorial

	// Entry 1 - miss program
	//TODO: Support binding necessary data
	for (int i = 0; i < missPrograms.size(); i++)
	{
		memcpy(pData + (mShaderTableEntrySize * (i + 1)), pRtsoProps->GetShaderIdentifier(missPrograms[i].missShader), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
	}

	for (auto& mesh : meshDB)
	{
		int i = 0;

		//For each mesh instance
		for (auto& instance : mesh.second->GetInstances())
		{

			if (i == 0)//(int)(mesh.second->GetInstances().size()) - 1)
			{
				int k = 0;
			}

			for (auto& hitProgram : instance.GetHitPrograms())
			{
				
				if (hitProgram->localRootSignature != nullptr)
				{
				}
			}
			i++;

		}
	}

	//Bind each VBO to a shader entry
	int counter = 0;
	for (auto& mesh : meshDB)
	{
		//For each mesh instance
		for (auto& instance : mesh.second->GetInstances())
		{
			for (auto& hitProgram : instance.GetHitPrograms())
			{
				// Entry 2 - hit program
				uint8_t* pHitEntry = pData + mShaderTableEntrySize * (missPrograms.size() + 1 + counter); // +3 skips the ray-gen and miss entries
				memcpy(pHitEntry, pRtsoProps->GetShaderIdentifier(hitProgram->exportName.c_str()), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

				//Bind data
				if (hitProgram->localRootSignature != nullptr)
				{
					uint8_t* pCbDesc = pHitEntry + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
					assert(((uint64_t)pCbDesc % 8) == 0); // Root descriptor must be stored at an 8-byte aligned address

					*(D3D12_GPU_VIRTUAL_ADDRESS*)pCbDesc = mesh.second->GetVBOs()[0]->GetGPUVirtualAddress();

					pCbDesc += 8; //Wow this actually worked
					*(D3D12_GPU_VIRTUAL_ADDRESS*)pCbDesc = mesh.second->GetIndices()[0]->GetGPUVirtualAddress();

					pCbDesc += 8;
					*(D3D12_GPU_VIRTUAL_ADDRESS*)pCbDesc = tlas.pResult->GetGPUVirtualAddress();

					for(int i = 0; i < instance.GetResources().size(); i++)
					{
						pCbDesc += 8;
						*(D3D12_GPU_VIRTUAL_ADDRESS*)pCbDesc = instance.GetResources()[i]->GetGPUVirtualAddress();
					}
				}

				counter++;
			}
		}
	}

	// Unmap
	mpShaderTable->Unmap(0, nullptr);
}
