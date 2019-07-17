#include "Renderer.h"
#include <vector>
#include "Mesh.h"
#include <map>
#include "ResourceManager.h"
Renderer* Renderer::mInstance = nullptr;

const D3D12_HEAP_PROPERTIES Renderer::kUploadHeapProps =
{
	D3D12_HEAP_TYPE_UPLOAD,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0,
};

const D3D12_HEAP_PROPERTIES Renderer::kDefaultHeapProps =
{
	D3D12_HEAP_TYPE_DEFAULT,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0
};

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







ID3D12Resource* Renderer::CreateVertexBuffer(const std::vector<vec3>& verts)
{
	// For simplicity, we create the vertex buffer on the upload heap, but that's not required
	ID3D12ResourcePtr pBuffer = RendererUtil::CreateBuffer(mWinHandle, mpDevice, verts.size(), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, kUploadHeapProps);
	uint8_t* pData;
	pBuffer->Map(0, nullptr, (void**)& pData);
	memcpy(pData, verts.data(), verts.size());
	pBuffer->Unmap(0, nullptr);
	return pBuffer;
}


Renderer::AccelerationStructureBuffers Renderer::CreateBLAS(std::shared_ptr<Mesh> mesh)
{
	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geomDesc;

	auto vbos = mesh->GetVBOs();
	uint32_t geometryCount = vbos.size();
	geomDesc.resize(geometryCount);

	//Setup descriptors for each geometry in the Mesh
	auto vertexCount = mesh->GetVertexCounts();
	for (uint32_t i = 0; i < geometryCount; i++)
	{
		geomDesc[i].Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geomDesc[i].Triangles.VertexBuffer.StartAddress = vbos[i]->GetGPUVirtualAddress();
		geomDesc[i].Triangles.VertexBuffer.StrideInBytes = sizeof(vec3);
		geomDesc[i].Triangles.VertexCount = vertexCount[i];
		geomDesc[i].Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
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
	buffers.pScratch = RendererUtil::CreateBuffer(mWinHandle, mpDevice, info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, kDefaultHeapProps);
	buffers.pResult = RendererUtil::CreateBuffer(mWinHandle, mpDevice, info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, kDefaultHeapProps);

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
	// First, get the size of the TLAS buffers and create them
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	inputs.NumDescs = 3;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	//This will be used to find out the potential size of the memory that we can make use of on the GPU
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
	mpDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

	//Get instance count
	int totalInstanceCount = 0;

	for (auto mesh : meshDB)
	{
		totalInstanceCount += mesh.second->GetInstanceCount();
	}

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
		buffers.pScratch = RendererUtil::CreateBuffer(mWinHandle, mpDevice, info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, kDefaultHeapProps);
		buffers.pResult = RendererUtil::CreateBuffer(mWinHandle, mpDevice, info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, kDefaultHeapProps);
		buffers.pInstanceDesc = RendererUtil::CreateBuffer(mWinHandle, mpDevice, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * totalInstanceCount, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, kUploadHeapProps);
		tlasSize = info.ResultDataMaxSizeInBytes;
	}


	

	// Map the instance desc buffer
	D3D12_RAYTRACING_INSTANCE_DESC* instanceDescs;
	buffers.pInstanceDesc->Map(0, nullptr, (void**)& instanceDescs);
	ZeroMemory(instanceDescs, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * totalInstanceCount);


	// The InstanceContributionToHitGroupIndex is set based on the shader-table layout specified in createShaderTable()

	int i = 0;
	for(auto mesh : meshDB)
	{
		auto& instances = mesh.second->GetInstances();
		auto blas = mesh.second->GetBLAS();

		for (const auto& instance : instances)
		{
			//A lot of this could be switched into a instance class...
			instanceDescs[i].InstanceID = i; // This value will be exposed to the shader via InstanceID()
			instanceDescs[i].InstanceContributionToHitGroupIndex = 0; //TODO:: UPDATE ME LATER FOR INTERESTING STUFF
			instanceDescs[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
			mat4 m = transpose(instance); // GLM is column major, the INSTANCE_DESC is row major
			memcpy(instanceDescs[i].Transform, &m, sizeof(instanceDescs[i].Transform));
			instanceDescs[i].AccelerationStructure = blas->GetGPUVirtualAddress();
			instanceDescs[i].InstanceMask = 0xFF;
			i++;
		}
		
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
		mesh.second->SetBLAS(CreateBLAS(mesh.second).pResult);
	}

	BuildTLAS(db, mTlasSize, false, mTopLevelBuffers);
	
	//TODO:: Think about below. 
	// The tutorial doesn't have any resource lifetime management, so we flush and sync here. This is not required by the DXR spec - you can submit the list whenever you like as long as you take care of the resources lifetime.
	mFenceValue = RendererUtil::SubmitCommandList(mpCmdList, mpCmdQueue, mpFence, mFenceValue);
	mpFence->SetEventOnCompletion(mFenceValue, mFenceEvent);
	WaitForSingleObject(mFenceEvent, INFINITE);
	uint32_t bufferIndex = mpSwapChain->GetCurrentBackBufferIndex();
	mpCmdList->Reset(mFrameObjects[0].pCmdAllocator, nullptr);
}



void Renderer::Render()
{
}

void Renderer::Shutdown()
{
}



