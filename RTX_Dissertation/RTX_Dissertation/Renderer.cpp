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
	mpDevice = CreateDevice(pDxgiFactory);
	mpCmdQueue = CreateCommandQueue();
	mpSwapChain = CreateDxgiSwapChain(pDxgiFactory, mSwapChainSize.x, mSwapChainSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, mpCmdQueue);



	// Create a Render Target View descriptor heap
	mRtvHeap.pHeap = CreateDescriptorHeap(k_RtvHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);

	// Create the per-frame objects
	for (uint32_t i = 0; i < arraysize(mFrameObjects); i++)
	{
		RendererUtil::D3DCall(mWinHandle,mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mFrameObjects[i].pCmdAllocator)));
		RendererUtil::D3DCall(mWinHandle, mpSwapChain->GetBuffer(i, IID_PPV_ARGS(&mFrameObjects[i].pSwapChainBuffer)));
		mFrameObjects[i].rtvHandle = CreateRTV(mFrameObjects[i].pSwapChainBuffer, mRtvHeap.pHeap, mRtvHeap.usedEntries, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	}

	// Create the command-list
	RendererUtil::D3DCall(mWinHandle, mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mFrameObjects[0].pCmdAllocator, nullptr, IID_PPV_ARGS(&mpCmdList)));

	// Create a fence and the event
	RendererUtil::D3DCall(mWinHandle, mpDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mpFence)));
	mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

}

ID3D12Device5Ptr Renderer::CreateDevice(IDXGIFactory4Ptr pDxgiFactory)
{
	// Find the HW adapter
	IDXGIAdapter1Ptr pAdapter;

	//Find correct 
	for (uint32_t i = 0; DXGI_ERROR_NOT_FOUND != pDxgiFactory->EnumAdapters1(i, &pAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 desc;
		pAdapter->GetDesc1(&desc);

		// Skip SW adapters
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
#ifdef _DEBUG
		ID3D12DebugPtr pDx12Debug;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDx12Debug))))
		{
			pDx12Debug->EnableDebugLayer();
		}
#endif
		// Create the device
		ID3D12Device5Ptr pDevice;
		RendererUtil::D3DCall(mWinHandle, D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice)));

		D3D12_FEATURE_DATA_D3D12_OPTIONS5 features5;
		HRESULT hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features5, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));
		if (FAILED(hr) || features5.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
		{
			RendererUtil::DisplayMessage(mWinHandle, "Raytracing is not supported on this device. Make sure your GPU supports DXR (such as Nvidia's Volta or Turing RTX) and you're on the latest drivers. The DXR fallback layer is not supported.");
			exit(1);
		}
		return pDevice;
	}
	return nullptr;
}

ID3D12CommandQueuePtr Renderer::CreateCommandQueue()
{
	ID3D12CommandQueuePtr pQueue;
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	RendererUtil::D3DCall(mWinHandle, mpDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&pQueue)));
	return pQueue;
}

IDXGISwapChain3Ptr Renderer::CreateDxgiSwapChain(IDXGIFactory4Ptr pFactory, uint32_t width, uint32_t height, DXGI_FORMAT format, ID3D12CommandQueuePtr pCommandQueue)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = 3;
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = format;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	// CreateSwapChainForHwnd() doesn't accept IDXGISwapChain3 (Why MS? Why?)
	MAKE_SMART_COM_PTR(IDXGISwapChain1);
	IDXGISwapChain1Ptr pSwapChain;

	HRESULT hr = pFactory->CreateSwapChainForHwnd(pCommandQueue, mWinHandle, &swapChainDesc, nullptr, nullptr, &pSwapChain);
	if (FAILED(hr))
	{
		RendererUtil::ShowD3DErrorMessage(mWinHandle, hr);
		return false;
	}

	IDXGISwapChain3Ptr pSwapChain3;
	RendererUtil::D3DCall(mWinHandle, pSwapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain3)));
	return pSwapChain3;
}

ID3D12DescriptorHeapPtr Renderer::CreateDescriptorHeap(uint32_t count, D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = count;
	desc.Type = type;
	desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ID3D12DescriptorHeapPtr pHeap;
	RendererUtil::D3DCall( mWinHandle,mpDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap)));
	return pHeap;
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::CreateRTV(ID3D12ResourcePtr pResource, ID3D12DescriptorHeapPtr pHeap, uint32_t& usedHeapEntries, DXGI_FORMAT format)
{
	D3D12_RENDER_TARGET_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	desc.Format = format;
	desc.Texture2D.MipSlice = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += usedHeapEntries * mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	usedHeapEntries++;
	mpDevice->CreateRenderTargetView(pResource, &desc, rtvHandle);
	return rtvHandle;
}

uint64_t Renderer::SubmitCommandList(ID3D12GraphicsCommandList4Ptr pCmdList, ID3D12CommandQueuePtr pCmdQueue, ID3D12FencePtr pFence, uint64_t fenceValue)
{
	pCmdList->Close();
	ID3D12CommandList* pGraphicsList = pCmdList.GetInterfacePtr();
	pCmdQueue->ExecuteCommandLists(1, &pGraphicsList);
	fenceValue++;
	pCmdQueue->Signal(pFence, fenceValue);
	return fenceValue;
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
	buffers.pScratch = CreateBuffer(info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, kDefaultHeapProps);
	buffers.pResult = CreateBuffer(info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, kDefaultHeapProps);

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
		buffers.pScratch = CreateBuffer(info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, kDefaultHeapProps);
		buffers.pResult = CreateBuffer(info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, kDefaultHeapProps);
		buffers.pInstanceDesc = CreateBuffer(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * 3, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, kUploadHeapProps);
		tlasSize = info.ResultDataMaxSizeInBytes;
	}


	//Get instance count
	int totalInstanceCount = 0;

	for (auto mesh : meshDB) 
	{
		totalInstanceCount += mesh.second->GetInstanceCount();
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
	mFenceValue = SubmitCommandList(mpCmdList, mpCmdQueue, mpFence, mFenceValue);
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

ID3D12ResourcePtr Renderer::CreateBuffer(size_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps)
{
	D3D12_RESOURCE_DESC bufDesc = {};
	bufDesc.Alignment = 0;
	bufDesc.DepthOrArraySize = 1;
	bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufDesc.Flags = flags;
	bufDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufDesc.Height = 1;
	bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufDesc.MipLevels = 1;
	bufDesc.SampleDesc.Count = 1;
	bufDesc.SampleDesc.Quality = 0;
	bufDesc.Width = size;

	ID3D12ResourcePtr pBuffer;
	RendererUtil::D3DCall(mWinHandle, mpDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc, initState, nullptr, IID_PPV_ARGS(&pBuffer)));
	return pBuffer;
}

ID3D12Resource* Renderer::CreateVertexBuffer(const std::vector<vec3>& verts)
{
	// For simplicity, we create the vertex buffer on the upload heap, but that's not required
	ID3D12ResourcePtr pBuffer = CreateBuffer(verts.size(), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, kUploadHeapProps);
	uint8_t* pData;
	pBuffer->Map(0, nullptr, (void**)& pData);
	memcpy(pData, verts.data(), verts.size());
	pBuffer->Unmap(0, nullptr);
	return pBuffer;
}

