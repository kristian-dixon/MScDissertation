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

#include "RendererUtil.h"


using namespace glm;



// Common DX12 definitions -- These are taken from NVIDIA's DXR Tutorials -- CRITICAL TODO:: INCLUDE LINK TO GITHUB
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

#define arraysize(a) (sizeof(a)/sizeof(a[0])) 
#define align_to(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment) 
// - End of NVIDA's code

//TODO:: REMOVE ME! ^^^

class Renderer
{
public:
	struct AccelerationStructureBuffers
	{
		ID3D12ResourcePtr pScratch;
		ID3D12ResourcePtr pResult;
		ID3D12ResourcePtr pInstanceDesc;    // Used only for top-level AS
	};

	struct HeapData
	{
		ID3D12DescriptorHeapPtr pHeap;
		uint32_t usedEntries;
	};



	static Renderer* CreateInstance(HWND winHandle, uint32_t winWidth, uint32_t winHeight);
	static Renderer* GetInstance() { return mInstance; };

	void Render();
	void Shutdown();
	ID3D12ResourcePtr CreateBuffer(size_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps);
	ID3D12ResourcePtr CreateVertexBuffer(const std::vector<vec3>& verts);

private:
	Renderer(HWND winHandle, uint32_t winWidth, uint32_t winHeight);
	void InitDXR();

	ID3D12Device5Ptr CreateDevice(IDXGIFactory4Ptr pDxgiFactory);

	ID3D12CommandQueuePtr CreateCommandQueue();
	IDXGISwapChain3Ptr CreateDxgiSwapChain(IDXGIFactory4Ptr pFactory, uint32_t width, uint32_t height, DXGI_FORMAT format, ID3D12CommandQueuePtr pCommandQueue);
	ID3D12DescriptorHeapPtr CreateDescriptorHeap(uint32_t count, D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible);

	struct
	{
		ID3D12CommandAllocatorPtr pCmdAllocator;
		ID3D12ResourcePtr pSwapChainBuffer;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	} mFrameObjects[3];

	D3D12_CPU_DESCRIPTOR_HANDLE CreateRTV(ID3D12ResourcePtr pResource, ID3D12DescriptorHeapPtr pHeap, uint32_t& usedHeapEntries, DXGI_FORMAT format);

//AccelerationStructureBuffers CreateBLAS();

//Properties
private:
	static Renderer* mInstance;

	HWND mWinHandle;
	uvec2 mSwapChainSize;
	ID3D12Device5Ptr mpDevice;
	ID3D12CommandQueuePtr mpCmdQueue;
	IDXGISwapChain3Ptr mpSwapChain;
	ID3D12GraphicsCommandList4Ptr mpCmdList;
	
	//Render target view heap
	HeapData mRtvHeap;

	ID3D12FencePtr mpFence;
	HANDLE mFenceEvent;

	//CONSTANTS
	static const D3D12_HEAP_PROPERTIES kUploadHeapProps;
	static const D3D12_HEAP_PROPERTIES kDefaultHeapProps;
	static const uint32_t k_RtvHeapSize;
	//static const uint32_t kDefaultSwapChainBuffers ;

};

