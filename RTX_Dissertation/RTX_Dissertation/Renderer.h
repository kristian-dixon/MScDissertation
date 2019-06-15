#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define GLM_FORCE_CTOR_INIT
#include "Externals/GLM/glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "Externals/GLM/glm/gtx/transform.hpp"
#include "Externals/GLM/glm/gtx/euler_angles.hpp"
#include <string>
#include <d3d12.h>
#include <comdef.h>
#include <dxgi1_4.h>
#include <dxgiformat.h>
#include <fstream>
#include "Externals/DXCAPI/dxcapi.use.h"
#include <vector>
#include <array>
#include <cstdint>

using namespace glm;

HWND gWinHandle2 = nullptr;


// Common DX12 definitions
#define MAKE_SMART_COM_PTR(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))
MAKE_SMART_COM_PTR(ID3D12Device5);
MAKE_SMART_COM_PTR(ID3D12GraphicsCommandList4);
MAKE_SMART_COM_PTR(ID3D12CommandQueue);
MAKE_SMART_COM_PTR(IDXGISwapChain3);
MAKE_SMART_COM_PTR(IDXGIFactory4);
MAKE_SMART_COM_PTR(IDXGIAdapter1);
MAKE_SMART_COM_PTR(ID3D12Fence);
MAKE_SMART_COM_PTR(ID3D12CommandAllocator);
MAKE_SMART_COM_PTR(ID3D12Resource);
MAKE_SMART_COM_PTR(ID3D12DescriptorHeap);
MAKE_SMART_COM_PTR(ID3D12Debug);
MAKE_SMART_COM_PTR(ID3D12StateObject);
MAKE_SMART_COM_PTR(ID3D12RootSignature);
MAKE_SMART_COM_PTR(ID3DBlob);
MAKE_SMART_COM_PTR(IDxcBlobEncoding);

static const uint32_t kDefaultSwapChainBuffers = 3;


#define d3d_call(a) {HRESULT hr_ = a; if(FAILED(hr_)) { d3dTraceHR( #a, hr_); }}
#define arraysize(a) (sizeof(a)/sizeof(a[0]))
#define align_to(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)


void d3dTraceHR(const std::string& msg, HRESULT hr);



class Renderer
{
public:
	void Init(HWND winHandle, uint32_t winWidth, uint32_t winHeight);
	void Render();
	void Shutdown();


private:
	IDXGISwapChain3Ptr createDxgiSwapChain(IDXGIFactory4Ptr pFactory, HWND hwnd, uint32_t width, uint32_t height, DXGI_FORMAT format, ID3D12CommandQueuePtr pCommandQueue);
	ID3D12Device5Ptr createDevice(IDXGIFactory4Ptr pDxgiFactory);
	ID3D12CommandQueuePtr createCommandQueue(ID3D12Device5Ptr pDevice);
	ID3D12DescriptorHeapPtr createDescriptorHeap(ID3D12Device5Ptr pDevice, uint32_t count, D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible);
	D3D12_CPU_DESCRIPTOR_HANDLE createRTV(ID3D12Device5Ptr pDevice, ID3D12ResourcePtr pResource, ID3D12DescriptorHeapPtr pHeap, uint32_t& usedHeapEntries, DXGI_FORMAT format);
	void resourceBarrier(ID3D12GraphicsCommandList4Ptr pCmdList, ID3D12ResourcePtr pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
	uint64_t submitCommandList(ID3D12GraphicsCommandList4Ptr pCmdList, ID3D12CommandQueuePtr pCmdQueue, ID3D12FencePtr pFence, uint64_t fenceValue);

	
	void initDXR(HWND winHandle, uint32_t winWidth, uint32_t winHeight);
	uint32_t beginFrame();
	void endFrame(uint32_t rtvIndex);





	HWND mHwnd = nullptr;
	ID3D12Device5Ptr mpDevice;
	ID3D12CommandQueuePtr mpCmdQueue;
	IDXGISwapChain3Ptr mpSwapChain;
	uvec2 mSwapChainSize;
	ID3D12GraphicsCommandList4Ptr mpCmdList;
	ID3D12FencePtr mpFence;
	HANDLE mFenceEvent;
	uint64_t mFenceValue = 0;

	struct
	{
		ID3D12CommandAllocatorPtr pCmdAllocator;
		ID3D12ResourcePtr pSwapChainBuffer;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	} mFrameObjects[kDefaultSwapChainBuffers];


	// Heap data
	struct HeapData
	{
		ID3D12DescriptorHeapPtr pHeap;
		uint32_t usedEntries = 0;
	};
	HeapData mRtvHeap;
	static const uint32_t kRtvHeapSize = 3;
};

