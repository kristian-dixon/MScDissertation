#include "Renderer.h"
#include  "d3d12.h"

HWND Renderer::gWinHandle2 = nullptr;

void msgBox2(const std::string& msg)
{
	MessageBoxA(Renderer::gWinHandle2, msg.c_str(), "Error", MB_OK);
}

void d3dTraceHR(const std::string& msg, HRESULT hr)
{
	char hr_msg[512];
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, hr, 0, hr_msg, ARRAYSIZE(hr_msg), nullptr);

	std::string error_msg = msg + ".\nError! " + hr_msg;
	msgBox2(error_msg);
}



void Renderer::initDXR(HWND winHandle, uint32_t winWidth, uint32_t winHeight)
{
	mHwnd = winHandle;
	gWinHandle2 = winHandle;
	mSwapChainSize = uvec2(winWidth, winHeight);

	// Initialize the debug layer for debug builds
#ifdef _DEBUG
	ID3D12DebugPtr pDebug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug))))
	{
		pDebug->EnableDebugLayer();
	}
#endif
	// Create the DXGI factory
	IDXGIFactory4Ptr pDxgiFactory;
	d3d_call(CreateDXGIFactory1(IID_PPV_ARGS(&pDxgiFactory)));
	mpDevice = createDevice(pDxgiFactory);
	mpCmdQueue = createCommandQueue(mpDevice);
	mpSwapChain = createDxgiSwapChain(pDxgiFactory, mHwnd, winWidth, winHeight, DXGI_FORMAT_R8G8B8A8_UNORM, mpCmdQueue);

	// Create a RTV descriptor heap
	mRtvHeap.pHeap = createDescriptorHeap(mpDevice, kRtvHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);

	// Create the per-frame objects
	for (uint32_t i = 0; i < kDefaultSwapChainBuffers; i++)
	{
		d3d_call(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mFrameObjects[i].pCmdAllocator)));
		d3d_call(mpSwapChain->GetBuffer(i, IID_PPV_ARGS(&mFrameObjects[i].pSwapChainBuffer)));
		mFrameObjects[i].rtvHandle = createRTV(mpDevice, mFrameObjects[i].pSwapChainBuffer, mRtvHeap.pHeap, mRtvHeap.usedEntries, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	}

	// Create the command-list
	d3d_call(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mFrameObjects[0].pCmdAllocator, nullptr, IID_PPV_ARGS(&mpCmdList)));

	// Create a fence and the event
	d3d_call(mpDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mpFence)));
	mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

IDXGISwapChain3Ptr Renderer::createDxgiSwapChain(IDXGIFactory4Ptr pFactory, HWND hwnd, uint32_t width, uint32_t height,
	DXGI_FORMAT format, ID3D12CommandQueuePtr pCommandQueue)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = kDefaultSwapChainBuffers;
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = format;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	// CreateSwapChainForHwnd() doesn't accept IDXGISwapChain3 (Why MS? Why?)
	MAKE_SMART_COM_PTR(IDXGISwapChain1);
	IDXGISwapChain1Ptr pSwapChain;

	HRESULT hr = pFactory->CreateSwapChainForHwnd(pCommandQueue, hwnd, &swapChainDesc, nullptr, nullptr, &pSwapChain);
	if (FAILED(hr))
	{
		d3dTraceHR("Failed to create the swap-chain", hr);
		return false;
	}

	IDXGISwapChain3Ptr pSwapChain3;
	d3d_call(pSwapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain3)));
	return pSwapChain3;
}

ID3D12Device5Ptr Renderer::createDevice(IDXGIFactory4Ptr pDxgiFactory)
{
	// Find the HW adapter
	IDXGIAdapter1Ptr pAdapter;

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
		//d3d_call(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice)));

		D3D12_FEATURE_DATA_D3D12_OPTIONS5 features5;
		HRESULT hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features5, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));
		if (FAILED(hr) || features5.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
		{
			msgBox2("Raytracing is not supported on this device. Make sure your GPU supports DXR (such as Nvidia's Volta or Turing RTX) and you're on the latest drivers. The DXR fallback layer is not supported.");
			exit(1);
		}
		return pDevice;
	}
	return nullptr;
}

ID3D12CommandQueuePtr Renderer::createCommandQueue(ID3D12Device5Ptr pDevice)
{
	ID3D12CommandQueuePtr pQueue;
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	d3d_call(pDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&pQueue)));
	return pQueue;
}

ID3D12DescriptorHeapPtr Renderer::createDescriptorHeap(ID3D12Device5Ptr pDevice, uint32_t count,
	D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = count;
	desc.Type = type;
	desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ID3D12DescriptorHeapPtr pHeap;
	d3d_call(pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap)));
	return pHeap;
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::createRTV(ID3D12Device5Ptr pDevice, ID3D12ResourcePtr pResource,
	ID3D12DescriptorHeapPtr pHeap, uint32_t& usedHeapEntries, DXGI_FORMAT format)
{
	D3D12_RENDER_TARGET_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	desc.Format = format;
	desc.Texture2D.MipSlice = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += usedHeapEntries * pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	usedHeapEntries++;
	pDevice->CreateRenderTargetView(pResource, &desc, rtvHandle);
	return rtvHandle;
}

void Renderer::resourceBarrier(ID3D12GraphicsCommandList4Ptr pCmdList, ID3D12ResourcePtr pResource,
	D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = pResource;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = stateBefore;
	barrier.Transition.StateAfter = stateAfter;
	pCmdList->ResourceBarrier(1, &barrier);
}

uint64_t Renderer::submitCommandList(ID3D12GraphicsCommandList4Ptr pCmdList, ID3D12CommandQueuePtr pCmdQueue,
	ID3D12FencePtr pFence, uint64_t fenceValue)
{
	pCmdList->Close();
	ID3D12CommandList* pGraphicsList = pCmdList.GetInterfacePtr();
	pCmdQueue->ExecuteCommandLists(1, &pGraphicsList);
	fenceValue++;
	pCmdQueue->Signal(pFence, fenceValue);
	return fenceValue;
}


uint32_t Renderer::beginFrame()
{
	return mpSwapChain->GetCurrentBackBufferIndex();

}

void Renderer::endFrame(uint32_t rtvIndex)
{
	resourceBarrier(mpCmdList, mFrameObjects[rtvIndex].pSwapChainBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mFenceValue = submitCommandList(mpCmdList, mpCmdQueue, mpFence, mFenceValue);
	mpSwapChain->Present(0, 0);

	// Prepare the command list for the next frame
	uint32_t bufferIndex = mpSwapChain->GetCurrentBackBufferIndex();

	// Make sure we have the new back-buffer is ready
	if (mFenceValue > kDefaultSwapChainBuffers)
	{
		mpFence->SetEventOnCompletion(mFenceValue - kDefaultSwapChainBuffers + 1, mFenceEvent);
		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	mFrameObjects[bufferIndex].pCmdAllocator->Reset();
	mpCmdList->Reset(mFrameObjects[bufferIndex].pCmdAllocator, nullptr);
}


void Renderer::Init(HWND winHandle, uint32_t winWidth, uint32_t winHeight)
{
	initDXR(winHandle, winWidth, winHeight); // Tutorial 02

}

void Renderer::Render()
{
	uint32_t rtvIndex = beginFrame();
	const float clearColor[4] = { 0.4f, 0.6f, 0.2f, 1.0f };
	resourceBarrier(mpCmdList, mFrameObjects[rtvIndex].pSwapChainBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mpCmdList->ClearRenderTargetView(mFrameObjects[rtvIndex].rtvHandle, clearColor, 0, nullptr);
	endFrame(rtvIndex);
}

void Renderer::Shutdown()
{
	// Wait for the command queue to finish execution
	mFenceValue++;
	mpCmdQueue->Signal(mpFence, mFenceValue);
	mpFence->SetEventOnCompletion(mFenceValue, mFenceEvent);
	WaitForSingleObject(mFenceEvent, INFINITE);
}
