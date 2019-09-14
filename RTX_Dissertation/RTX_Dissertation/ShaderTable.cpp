#include "ShaderTable.h"
#include "ResourceManager.h"

void ShaderTable::BuildShaderTable(HWND windowHandle, ID3D12Device5Ptr device, ID3D12StateObjectPtr mpPipelineState, AccelerationStructureBuffers& tlas, ID3D12DescriptorHeapPtr mpSrvUavHeap)
{
	const WCHAR* kRayGenShader = L"rayGen";

	// TODO:: Make this nicer
	mShaderTableEntrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	mShaderTableEntrySize += 32 + 64; // The ray-gen's descriptor table
	mShaderTableEntrySize = align_to(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, mShaderTableEntrySize);

	//Raygen 
	int entryCount = 1;

	entryCount += static_cast<int>(missPrograms.size());

	auto meshDB = ResourceManager::GetMeshDB();
	int largestHitGroupSize = 0;
	int hitGroupsCount = 0;
	for (auto& mesh : meshDB)
	{
		for (auto& instances : mesh.second->GetInstances())
		{
			entryCount += static_cast<int>(instances.GetHitPrograms().size());

			/*if (instances.GetHitPrograms().size() > largestHitGroupSize)
			{
				largestHitGroupSize = instances.GetHitPrograms().size();
			}*/
		}
	}
	//entryCount += largestHitGroupSize * hitGroupsCount;

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

	// Entry 1 - miss program
	//TODO: Support binding necessary data
	for (int i = 0; i < missPrograms.size(); i++)
	{
		uint8_t* pMissEntry = pData + (mShaderTableEntrySize * (i + 1));
		memcpy(pMissEntry, pRtsoProps->GetShaderIdentifier(missPrograms[i]->missShader), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

		uint8_t* pCbDesc = pMissEntry + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		assert(((uint64_t)pCbDesc % 8) == 0); // Root descriptor must be stored at an 8-byte aligned address
		
		for(int j = 0; j < missPrograms[i]->boundData.size(); j++)
		{
			*(D3D12_GPU_VIRTUAL_ADDRESS*)pCbDesc = missPrograms[i]->boundData[j]->GetGPUVirtualAddress();
			pCbDesc += 8;
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

					pCbDesc += 8;
					*(D3D12_GPU_VIRTUAL_ADDRESS*)pCbDesc = instance.GetTransformCB()->GetGPUVirtualAddress();

					for(int i = 0; i < instance.GetResources().size(); i++)
					{
						pCbDesc += 8;
						*(D3D12_GPU_VIRTUAL_ADDRESS*)pCbDesc = instance.GetResources()[i]->GetGPUVirtualAddress();
					}
				}

				counter++;
			}

			//Skip empty hitgroups
			//int skip = largestHitGroupSize - instance.GetHitPrograms().size();
			//counter += skip;
		}
	}

	// Unmap
	mpShaderTable->Unmap(0, nullptr);
}
