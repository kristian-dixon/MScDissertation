#include "Renderer.h"
#include <vector>
#include "Mesh.h"
#include <map>
#include "ResourceManager.h"
#include <DirectXMath.h>
#include "RaytracingPipelineState.h"
Renderer* Renderer::mInstance = nullptr;



const uint32_t Renderer::k_RtvHeapSize = 3;

Renderer* Renderer::CreateInstance(HWND winHandle, uint32_t winWidth, uint32_t winHeight)
{
	if (mInstance)
	{
		//Might not be a good idea but we need to have a way to say "DON'T TRY TO CREATE MULTIPLE RENDERERS"
		throw;
	}
	else
	{
		mInstance = new Renderer(winHandle, winWidth, winHeight);
	}
	return mInstance;
}



Renderer::Renderer(HWND winHandle, uint32_t winWidth, uint32_t winHeight) : mWinHandle(winHandle), mSwapChainSize(winWidth, winHeight)
{
}


void Renderer::InitDXR()
{
	//Initialize the debug layer
	#ifdef _DEBUG

	ID3D12DebugPtr pDebug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug))))
	{
		pDebug->EnableDebugLayer();
	}
	
	#endif 

	// Create the DXGI factory
	IDXGIFactory4Ptr pDxgiFactory;
	RendererUtil::D3DCall(mWinHandle, CreateDXGIFactory1(IID_PPV_ARGS(&pDxgiFactory)));
	mpDevice = RendererUtil::CreateDevice(mWinHandle, pDxgiFactory);
	mpCmdQueue = RendererUtil::CreateCommandQueue(mWinHandle, mpDevice);
	mpSwapChain = RendererUtil::CreateDxgiSwapChain(mWinHandle, pDxgiFactory, mSwapChainSize.x, mSwapChainSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, mpCmdQueue);

	

	// Create a Render Target View descriptor heap
	mRtvHeap.pHeap = RendererUtil::CreateDescriptorHeap(mWinHandle, mpDevice, k_RtvHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);
	// Create the per-frame objects
	for (uint32_t i = 0; i < arraysize(mFrameObjects); i++)
	{
		RendererUtil::D3DCall(mWinHandle,mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mFrameObjects[i].pCmdAllocator)));
		RendererUtil::D3DCall(mWinHandle, mpSwapChain->GetBuffer(i, IID_PPV_ARGS(&mFrameObjects[i].pSwapChainBuffer)));
		mFrameObjects[i].rtvHandle = RendererUtil::CreateRTV(mpDevice, mFrameObjects[i].pSwapChainBuffer, mRtvHeap.pHeap, mRtvHeap.usedEntries, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	}

	// Create the command-list
	RendererUtil::D3DCall(mWinHandle, mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mFrameObjects[0].pCmdAllocator, nullptr, IID_PPV_ARGS(&mpCmdList)));

	// Create a fence and the event
	RendererUtil::D3DCall(mWinHandle, mpDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mpFence)));
	mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

}

ID3D12ResourcePtr Renderer::CreateVertexBuffer(const std::vector<Vertex>& verts)
{
	// For simplicity, we create the vertex buffer on the upload heap, but that's not required
	auto test = sizeof(*verts.data());
	ID3D12ResourcePtr pBuffer = RendererUtil::CreateBuffer(mWinHandle, mpDevice,  sizeof(Vertex) * verts.size(), 
		D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, RendererUtil::kUploadHeapProps);
	uint8_t* pData;
	pBuffer->Map(0, nullptr, (void**)& pData);
	memcpy(pData, &verts[0], verts.size() * sizeof(Vertex));
	pBuffer->Unmap(0, nullptr);
	return pBuffer;
}


ID3D12ResourcePtr Renderer::CreateIndexBuffer(const std::vector<uint32_t>& verts)
{
	// For simplicity, we create the vertex buffer on the upload heap, but that's not required
	auto test = sizeof(*verts.data());
	ID3D12ResourcePtr pBuffer = RendererUtil::CreateBuffer(mWinHandle, mpDevice, verts.size() * sizeof(uint32_t), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, RendererUtil::kUploadHeapProps);
	uint8_t* pData;
	pBuffer->Map(0, nullptr, (void**)& pData);
	memcpy(pData, &verts[0], verts.size() * sizeof(uint32_t));
	pBuffer->Unmap(0, nullptr);
	return pBuffer;
}

AccelerationStructureBuffers Renderer::CreateBLAS(std::shared_ptr<Mesh> mesh)
{
	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geomDesc;

	auto& vbos = mesh->GetVBOs();
	auto& vertexCount = mesh->GetVertexCounts();

	auto& indices = mesh->GetIndices();
	auto& indexCounts = mesh->GetIndexCounts();

	auto indicesSize = indices.size();
	uint32_t geometryCount = vbos.size();
	geomDesc.resize(geometryCount);

	//Setup descriptors for each geometry in the Mesh
	for (uint32_t i = 0; i < geometryCount; i++)
	{
		geomDesc[i].Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geomDesc[i].Triangles.VertexBuffer.StartAddress = vbos[i]->GetGPUVirtualAddress();
		geomDesc[i].Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
		geomDesc[i].Triangles.VertexCount = vertexCount[i];
		geomDesc[i].Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

		if(i < indicesSize)
		{
			geomDesc[i].Triangles.IndexBuffer = indices[i]->GetGPUVirtualAddress();
			geomDesc[i].Triangles.IndexCount = mesh->GetIndexCounts()[i];
			geomDesc[i].Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
		}

		geomDesc[i].Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	}


	// Get the size requirements for the scratch and AS buffers
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = geometryCount;
	inputs.pGeometryDescs = geomDesc.data();
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
	mpDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);


	// Create the buffers. They need to support UAV, and since we are going to immediately use them, we create them with an unordered-access state
	AccelerationStructureBuffers buffers;
	buffers.pScratch = RendererUtil::CreateBuffer(mWinHandle, mpDevice, info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, RendererUtil::kDefaultHeapProps);
	buffers.pResult = RendererUtil::CreateBuffer(mWinHandle, mpDevice, info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, RendererUtil::kDefaultHeapProps);

	// Create the bottom-level AS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
	asDesc.Inputs = inputs;
	asDesc.DestAccelerationStructureData = buffers.pResult->GetGPUVirtualAddress();
	asDesc.ScratchAccelerationStructureData = buffers.pScratch->GetGPUVirtualAddress();

	mpCmdList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	// We need to insert a UAV barrier before using the acceleration structures in a raytracing operation
	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = buffers.pResult;
	mpCmdList->ResourceBarrier(1, &uavBarrier);

	

	return buffers;

}

void Renderer::BuildTLAS(const std::map<std::string, std::shared_ptr<Mesh>>& meshDB, uint64_t& tlasSize, bool update, AccelerationStructureBuffers& buffers)
{
	//Get instance count
	int totalInstanceCount = 0;

	for (auto mesh : meshDB)
	{
		totalInstanceCount += mesh.second->GetInstanceCount();
	}

	// First, get the size of the TLAS buffers and create them
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	inputs.NumDescs = totalInstanceCount; 
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	//This will be used to find out the potential size of the memory that we can make use of on the GPU
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
	mpDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

	

	if (update)
	{
		// If this a request for an update, then the TLAS was already used in a DispatchRay() call. We need a UAV barrier to make sure the read operation ends before updating the buffer
		D3D12_RESOURCE_BARRIER uavBarrier = {};
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = buffers.pResult;
		mpCmdList->ResourceBarrier(1, &uavBarrier);
	}
	else
	{
		// If this is not an update operation then we need to create the buffers, otherwise we will refit in-place
		buffers.pScratch = RendererUtil::CreateBuffer(mWinHandle, mpDevice, info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, RendererUtil::kDefaultHeapProps);
		buffers.pResult = RendererUtil::CreateBuffer(mWinHandle, mpDevice, info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, RendererUtil::kDefaultHeapProps);
		buffers.pInstanceDesc = RendererUtil::CreateBuffer(mWinHandle, mpDevice, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * totalInstanceCount, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, RendererUtil::kUploadHeapProps);
		tlasSize = info.ResultDataMaxSizeInBytes;
	}


	

	// Map the instance desc buffer
	D3D12_RAYTRACING_INSTANCE_DESC* instanceDescs;
	buffers.pInstanceDesc->Map(0, nullptr, (void**)& instanceDescs);
	ZeroMemory(instanceDescs, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * totalInstanceCount);


	// The InstanceContributionToHitGroupIndex is set based on the shader-table layout specified in createShaderTable()

	int instanceIndex = 0;
	int meshIndex = 0;
	for(auto mesh : meshDB)
	{
		auto& instances = mesh.second->GetInstances();
		auto blas = mesh.second->GetBLAS();

		for (auto& instance : instances)
		{
			//A lot of this could be switched into a instance class...
			instanceDescs[instanceIndex].InstanceID = instanceIndex; // This value will be exposed to the shader via InstanceID()
			instanceDescs[instanceIndex].InstanceContributionToHitGroupIndex = (meshIndex * 2) ;  //TODO:: Instead of this, have some count of the number of hit groups being called and use that instead
			instanceDescs[instanceIndex].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
			mat4 m = transpose(instance.GetTransform()); // GLM is column major, the INSTANCE_DESC is row major
			memcpy(instanceDescs[instanceIndex].Transform, &m, sizeof(instanceDescs[instanceIndex].Transform));
			instanceDescs[instanceIndex].AccelerationStructure = blas.pResult->GetGPUVirtualAddress();
			instanceDescs[instanceIndex].InstanceMask = 0xFF;
			instanceIndex++;
		}
		meshIndex++;
		
	}

	// Unmap
	buffers.pInstanceDesc->Unmap(0, nullptr);


	// Create the TLAS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
	asDesc.Inputs = inputs;
	asDesc.Inputs.InstanceDescs = buffers.pInstanceDesc->GetGPUVirtualAddress();
	asDesc.DestAccelerationStructureData = buffers.pResult->GetGPUVirtualAddress();
	asDesc.ScratchAccelerationStructureData = buffers.pScratch->GetGPUVirtualAddress();

	// If this is an update operation, set the source buffer and the perform_update flag
	if (update)
	{
		asDesc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
		asDesc.SourceAccelerationStructureData = buffers.pResult->GetGPUVirtualAddress();
	}

	mpCmdList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	// We need to insert a UAV barrier before using the acceleration structures in a raytracing operation
	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = buffers.pResult;
	mpCmdList->ResourceBarrier(1, &uavBarrier);

	
}


void Renderer::CreateAccelerationStructures()
{
	auto& db = ResourceManager::GetMeshDB();

	for( auto mesh : db)
	{
		mesh.second->SetBLAS(CreateBLAS(mesh.second));
	}

	

	BuildTLAS(db, mTlasSize, false, mTLAS);
	//TODO:: Think about below. 
	// The tutorial doesn't have any resource lifetime management, so we flush and sync here. This is not required by the DXR spec - you can submit the list whenever you like as long as you take care of the resources lifetime.
	mFenceValue = RendererUtil::SubmitCommandList(mpCmdList, mpCmdQueue, mpFence, mFenceValue);
	mpFence->SetEventOnCompletion(mFenceValue, mFenceEvent);
	WaitForSingleObject(mFenceEvent, INFINITE);
	uint32_t bufferIndex = mpSwapChain->GetCurrentBackBufferIndex();
	mpCmdList->Reset(mFrameObjects[0].pCmdAllocator, nullptr);
}



void Renderer::CreateShaderResources()
{
	// Create the output resource. The dimensions and format should match the swap-chain
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // The backbuffer is actually DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, but sRGB formats can't be used with UAVs. We will convert to sRGB ourselves in the shader
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	resDesc.Width = mSwapChainSize.x;
	resDesc.Height = mSwapChainSize.y;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	RendererUtil::D3DCall(mWinHandle, mpDevice->CreateCommittedResource(&RendererUtil::kDefaultHeapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&mpOutputResource))); // Starting as copy-source to simplify onFrameRender()

	// Create an SRV/UAV descriptor heap. Need 2 entries - 1 SRV for the scene and 1 UAV for the output and 1 for the camera cbv
	mpSrvUavHeap = RendererUtil::CreateDescriptorHeap(mWinHandle, mpDevice, 3, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

	// Create the UAV. Based on the root signature we created it should be the first entry
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	mpDevice->CreateUnorderedAccessView(mpOutputResource, nullptr, &uavDesc, mpSrvUavHeap->GetCPUDescriptorHandleForHeapStart());

	// Create the TLAS SRV right after the UAV. Note that we are using a different SRV desc here
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.RaytracingAccelerationStructure.Location = mTLAS.pResult->GetGPUVirtualAddress();
	
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = mpSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
	srvHandle.ptr += mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	mpDevice->CreateShaderResourceView(nullptr, &srvDesc, srvHandle);

	srvHandle.ptr += mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Describe and create a constant buffer view for the camera
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = mCamera.mCameraBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = mCamera.mCameraBufferSize;
	mpDevice->CreateConstantBufferView(&cbvDesc, srvHandle);

}


void Renderer::CreateShaderTable()
{
	//Here's how we're going to do this
	/*
	 * Entry 0 - Ray-Gen Program
	 * Entry 1 to MissShaderCount + 1 - Miss Shaders
	 
	 * Then for each object
	 *	- For Each Instance
	 *		- Bind Shaders
	 *		
	 * Profit.
	 */


	/** The shader-table layout is as follows:
		Entry 0 - Ray-gen program
		Entry 1 - Miss program
		Entry 2 - Hit program
		All entries in the shader-table must have the same size, so we will choose it base on the largest required entry.
		The ray-gen program requires the largest entry - sizeof(program identifier) + 8 bytes for a descriptor-table.
		The entry size must be aligned up to D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT
	*/
	const WCHAR* kRayGenShader = L"rayGen";
	const WCHAR* kMissShader = L"miss";
	const WCHAR* kHitGroup = L"HitGroup";
	const WCHAR* kShadowMiss = L"shadowMiss";
	const WCHAR* kShadowHitGroup = L"ShadowHitGroup";

	// Calculate the size and create the buffer
	mShaderTableEntrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	mShaderTableEntrySize += 24; // The ray-gen's descriptor table
	mShaderTableEntrySize = align_to(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, mShaderTableEntrySize);
	
	int vboCount = 0;
	auto meshDB = ResourceManager::GetMeshDB();
	for(auto& mesh : meshDB)
	{
		vboCount += mesh.second->GetVBOs().size();
	}


	uint32_t shaderTableSize = mShaderTableEntrySize * (3 + (vboCount * 2));

	// For simplicity, we create the shader-table on the upload heap. You can also create it on the default heap
	mpShaderTable = RendererUtil::CreateBuffer(mWinHandle, mpDevice, shaderTableSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, RendererUtil::kUploadHeapProps);

	// Map the buffer
	uint8_t* pData;
	RendererUtil::D3DCall(mWinHandle, mpShaderTable->Map(0, nullptr, (void**)& pData));

	MAKE_SMART_COM_PTR(ID3D12StateObjectProperties);
	ID3D12StateObjectPropertiesPtr pRtsoProps;
	mpPipelineState->QueryInterface(IID_PPV_ARGS(&pRtsoProps));

	// Entry 0 - ray-gen program ID and descriptor data
	memcpy(pData, pRtsoProps->GetShaderIdentifier(kRayGenShader), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
	uint64_t heapStart = mpSrvUavHeap->GetGPUDescriptorHandleForHeapStart().ptr;
	*(uint64_t*)(pData + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES) = heapStart;
	// This is where we need to set the descriptor data for the ray-gen shader. We'll get to it in the next tutorial

	// Entry 1 - miss program
	memcpy(pData + mShaderTableEntrySize, pRtsoProps->GetShaderIdentifier(kMissShader), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

	memcpy(pData + mShaderTableEntrySize * 2, pRtsoProps->GetShaderIdentifier(kShadowMiss), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);


	//Bind each VBO to a shader entry
	int counter = 0;
	for (auto& mesh : meshDB)
	{
		auto vbos = mesh.second->GetVBOs();
		//For each geometry
		for(int i = 0; i < vbos.size(); ++i)
		{
			
			{
				//Get instance info, mount 

				// Entry 2 - hit program
				uint8_t* pHitEntry = pData + mShaderTableEntrySize * (3 + counter); // +3 skips the ray-gen and miss entries
				memcpy(pHitEntry, pRtsoProps->GetShaderIdentifier(kHitGroup), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

				uint8_t* pCbDesc = pHitEntry + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
				assert(((uint64_t)pCbDesc % 8) == 0); // Root descriptor must be stored at an 8-byte aligned address

				*(D3D12_GPU_VIRTUAL_ADDRESS*)pCbDesc = vbos[i]->GetGPUVirtualAddress();

				pCbDesc += 8; //Wow this actually worked
				*(D3D12_GPU_VIRTUAL_ADDRESS*)pCbDesc = mesh.second->GetIndices()[i]->GetGPUVirtualAddress();

				pCbDesc += 8;
				*(D3D12_GPU_VIRTUAL_ADDRESS*)pCbDesc = mTLAS.pResult->GetGPUVirtualAddress();

			}

			counter++;

			//Bind shadow program
			{
				uint8_t* pHitEntry = pData + mShaderTableEntrySize * (3 + counter); // +3 skips the ray-gen and miss entries

				memcpy(pHitEntry, pRtsoProps->GetShaderIdentifier(kShadowHitGroup), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			}

			counter++;
		}
	}

	// Unmap
	mpShaderTable->Unmap(0, nullptr);
}

void Renderer::CreateDXRResources()
{
	CreateAccelerationStructures();   

	string shaderName = "Data/Shaders.hlsl";
	//CreateRTPipelineState();                   

	
	RaytracingPipelineState rtspo = RaytracingPipelineState(shaderName);

	{
		LocalRootSignature rgsRootSignature(mWinHandle, mpDevice, RendererUtil::CreateRayGenRootDesc().desc);

		auto hitGroups = ResourceManager::GetHitProgramDB();

		for (auto hitGroup : hitGroups)
		{
			rtspo.AddHitProgram(hitGroup.second);
		}

		/*auto hitGroup = HitProgram(nullptr, L"chs", L"HitGroup", &rgsRootSignature);
		auto hitGroup2 = HitProgram(nullptr, L"shadowChs", L"ShadowHitGroup");
		*/

		auto miss1 = MissProgram(L"miss");
		auto miss2 = MissProgram(L"shadowMiss");
		rtspo.AddMissProgram(miss1);
		rtspo.AddMissProgram(miss2);

		rtspo.BuildPipeline(mWinHandle, mpDevice);

		mpPipelineState = rtspo.GetPipelineObject();
	}
	
	mCamera.CreateCamera(mWinHandle, mpDevice);
	mCamera.UpdateCamera();

	CreateShaderResources();

	testCB = RendererUtil::CreateConstantBuffer(mWinHandle, mpDevice);

	CreateShaderTable();
}



uint32_t Renderer::BeginFrame()
{
	mCamera.UpdateCamera();
	
	// Bind the descriptor heaps
	ID3D12DescriptorHeap* heaps[] = { mpSrvUavHeap };
	mpCmdList->SetDescriptorHeaps(arraysize(heaps), heaps);
	return mpSwapChain->GetCurrentBackBufferIndex();
}

void Renderer::EndFrame(uint32_t rtvIndex)
{
	RendererUtil::ResourceBarrier(mpCmdList, mFrameObjects[rtvIndex].pSwapChainBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
	mFenceValue = RendererUtil::SubmitCommandList(mpCmdList, mpCmdQueue, mpFence, mFenceValue);
	mpSwapChain->Present(0, 0);

	// Prepare the command list for the next frame
	uint32_t bufferIndex = mpSwapChain->GetCurrentBackBufferIndex();

	// Make sure we have the new back-buffer is ready
	if (mFenceValue > 3) //Default swap chain buffer.     v too
	{
		mpFence->SetEventOnCompletion(mFenceValue - 3 + 1, mFenceEvent);
		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	mFrameObjects[bufferIndex].pCmdAllocator->Reset();
	mpCmdList->Reset(mFrameObjects[bufferIndex].pCmdAllocator, nullptr);
}


void Renderer::Render()
{
	uint32_t rtvIndex = BeginFrame();

	// Let's raytrace
	RendererUtil::ResourceBarrier(mpCmdList, mpOutputResource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	D3D12_DISPATCH_RAYS_DESC raytraceDesc = {};
	raytraceDesc.Width = mSwapChainSize.x;
	raytraceDesc.Height = mSwapChainSize.y;
	raytraceDesc.Depth = 1;

	// RayGen is the first entry in the shader-table
	raytraceDesc.RayGenerationShaderRecord.StartAddress = mpShaderTable->GetGPUVirtualAddress() + 0 * mShaderTableEntrySize;
	raytraceDesc.RayGenerationShaderRecord.SizeInBytes = mShaderTableEntrySize;

	// Miss is the second entry in the shader-table
	size_t missOffset = 1 * mShaderTableEntrySize;
	raytraceDesc.MissShaderTable.StartAddress = mpShaderTable->GetGPUVirtualAddress() + missOffset;
	raytraceDesc.MissShaderTable.StrideInBytes = mShaderTableEntrySize;
	raytraceDesc.MissShaderTable.SizeInBytes = mShaderTableEntrySize * 2;   // Only two miss-entries

	// Hit is the third entry in the shader-table
	size_t hitOffset = 3 * mShaderTableEntrySize;
	raytraceDesc.HitGroupTable.StartAddress = mpShaderTable->GetGPUVirtualAddress() + hitOffset;
	raytraceDesc.HitGroupTable.StrideInBytes = mShaderTableEntrySize;

	int vboCount = 0;
	auto meshDB = ResourceManager::GetMeshDB();
	for (auto& mesh : meshDB)
	{
		vboCount += mesh.second->GetVBOs().size();
	}
	raytraceDesc.HitGroupTable.SizeInBytes = mShaderTableEntrySize * (vboCount * 2);

	// Bind the empty root signature
	mpCmdList->SetComputeRootSignature(mpEmptyRootSig);

	// Dispatch
	mpCmdList->SetPipelineState1(mpPipelineState.GetInterfacePtr());
	mpCmdList->DispatchRays(&raytraceDesc);

	// Copy the results to the back-buffer
	RendererUtil::ResourceBarrier(mpCmdList, mpOutputResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	RendererUtil::ResourceBarrier(mpCmdList, mFrameObjects[rtvIndex].pSwapChainBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
	mpCmdList->CopyResource(mFrameObjects[rtvIndex].pSwapChainBuffer, mpOutputResource);

	EndFrame(rtvIndex);
}

void Renderer::Shutdown()
{
	mFenceValue++;
	mpCmdQueue->Signal(mpFence, mFenceValue);
	mpFence->SetEventOnCompletion(mFenceValue, mFenceEvent);
	WaitForSingleObject(mFenceEvent, INFINITE);
}



