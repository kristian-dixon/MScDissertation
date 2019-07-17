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
#include <memory>
#include <locale>
#include <codecvt>

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


#pragma once
class RendererUtil
{
public:
	static std::wstring string_2_wstring(const std::string& s)
	{
		std::wstring_convert<std::codecvt_utf8<WCHAR>> cvt;
		std::wstring ws = cvt.from_bytes(s);
		return ws;
	}
	static std::string wstring_2_string(const std::wstring& ws)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
		std::string s = cvt.to_bytes(ws);
		return s;
	}
	static void DisplayMessage(HWND winHandle, const std::string& msg, const std::string& type = "Error") { MessageBoxA(winHandle, msg.c_str(), type.c_str(), MB_OK); }
	static void D3DCall(HWND winHandle, HRESULT hr) { if (FAILED(hr)) { RendererUtil::ShowD3DErrorMessage(winHandle, hr); } }

	static void ShowD3DErrorMessage(HWND hwnd, HRESULT hr);

	static ID3D12Device5Ptr CreateDevice(HWND mWinHandle, IDXGIFactory4Ptr pDxgiFactory);

	static ID3D12CommandQueuePtr CreateCommandQueue(HWND mWinHandle, ID3D12Device5Ptr mpDevice);

	static IDXGISwapChain3Ptr CreateDxgiSwapChain(HWND mWinHandle, IDXGIFactory4Ptr pFactory, uint32_t width, uint32_t height, DXGI_FORMAT format, ID3D12CommandQueuePtr pCommandQueue);

	static ID3D12DescriptorHeapPtr CreateDescriptorHeap(HWND mWinHandle, ID3D12Device5Ptr mpDevice, uint32_t count, D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible);

	static D3D12_CPU_DESCRIPTOR_HANDLE CreateRTV(ID3D12Device5Ptr mpDevice, ID3D12ResourcePtr pResource, ID3D12DescriptorHeapPtr pHeap, uint32_t& usedHeapEntries, DXGI_FORMAT format);

	static ID3D12ResourcePtr CreateBuffer(HWND mWinHandle, ID3D12Device5Ptr mpDevice, size_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps);

	static uint64_t SubmitCommandList(ID3D12GraphicsCommandList4Ptr pCmdList, ID3D12CommandQueuePtr pCmdQueue, ID3D12FencePtr pFence, uint64_t fenceValue);

	
};

