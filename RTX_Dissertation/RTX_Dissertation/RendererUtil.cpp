#include "RendererUtil.h"
#include <comdef.h>

void RendererUtil::ShowD3DErrorMessage(HWND hwnd, HRESULT hr)
{
	auto message = _com_error(hr).ErrorMessage();
	std::string msg = wstring_2_string(message);

	char hr_msg[512];
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, hr, 0, hr_msg, ARRAYSIZE(hr_msg), nullptr);

	std::string error_msg = msg + ".\nError! " + hr_msg;
	DisplayMessage(hwnd, error_msg);
}



ID3D12Device5Ptr RendererUtil::CreateDevice(HWND mWinHandle, IDXGIFactory4Ptr pDxgiFactory)
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

ID3D12CommandQueuePtr RendererUtil::CreateCommandQueue(HWND mWinHandle, ID3D12Device5Ptr mpDevice)
{
	ID3D12CommandQueuePtr pQueue;
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	RendererUtil::D3DCall(mWinHandle, mpDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&pQueue)));
	return pQueue;
}

IDXGISwapChain3Ptr RendererUtil::CreateDxgiSwapChain(HWND mWinHandle, IDXGIFactory4Ptr pFactory, uint32_t width, uint32_t height, DXGI_FORMAT format, ID3D12CommandQueuePtr pCommandQueue)
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

ID3D12DescriptorHeapPtr RendererUtil::CreateDescriptorHeap(HWND mWinHandle, ID3D12Device5Ptr mpDevice, uint32_t count, D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = count;
	desc.Type = type;
	desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ID3D12DescriptorHeapPtr pHeap;
	RendererUtil::D3DCall(mWinHandle, mpDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap)));
	return pHeap;
}

D3D12_CPU_DESCRIPTOR_HANDLE RendererUtil::CreateRTV(ID3D12Device5Ptr mpDevice, ID3D12ResourcePtr pResource, ID3D12DescriptorHeapPtr pHeap, uint32_t& usedHeapEntries, DXGI_FORMAT format)
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

ID3D12ResourcePtr RendererUtil::CreateBuffer(HWND mWinHandle, ID3D12Device5Ptr mpDevice, size_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps)
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

uint64_t RendererUtil::SubmitCommandList(ID3D12GraphicsCommandList4Ptr pCmdList, ID3D12CommandQueuePtr pCmdQueue, ID3D12FencePtr pFence, uint64_t fenceValue)
{
	pCmdList->Close();
	ID3D12CommandList* pGraphicsList = pCmdList.GetInterfacePtr();
	pCmdQueue->ExecuteCommandLists(1, &pGraphicsList);
	fenceValue++;
	pCmdQueue->Signal(pFence, fenceValue);
	return fenceValue;
}