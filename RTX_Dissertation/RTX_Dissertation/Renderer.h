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

static const uint32_t kDefaultSwapChainBuffers = 3;


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

#define d3d_call(a) {HRESULT hr_ = a; if(FAILED(hr_)) { d3dTraceHR( #a, hr_); }}
#define arraysize(a) (sizeof(a)/sizeof(a[0]))
#define align_to(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)
// - End of NVIDA's code



class Renderer
{
public:
	static Renderer* m_instance;

	static Renderer* CreateInstance(HWND winHandle, uint32_t winWidth, uint32_t winHeight);


	void Render();
	void Shutdown();

private:
	Renderer(HWND winHandle, uint32_t winWidth, uint32_t winHeight);
	void Init(HWND winHandle, uint32_t winWidth, uint32_t winHeight);

};

