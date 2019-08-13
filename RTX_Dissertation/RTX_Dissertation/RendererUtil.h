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
#include <sstream>
#include <DirectXMath.h>


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

static dxc::DxcDllSupport gDxcDllHelper;
MAKE_SMART_COM_PTR(IDxcCompiler);
MAKE_SMART_COM_PTR(IDxcLibrary);
MAKE_SMART_COM_PTR(IDxcBlobEncoding);
MAKE_SMART_COM_PTR(IDxcOperationResult);

#define arraysize(a) (sizeof(a)/sizeof(a[0])) 
#define align_to(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment) 


struct AccelerationStructureBuffers
{
	ID3D12ResourcePtr pScratch;
	ID3D12ResourcePtr pResult;
	ID3D12ResourcePtr pInstanceDesc;    // Used only for top-level AS
};

struct HeapData
{
	ID3D12DescriptorHeapPtr pHeap;
	uint32_t usedEntries = 0;
};

struct DxilLibrary
{
	DxilLibrary(ID3DBlobPtr pBlob, const std::vector<const WCHAR*>& entryPoint, uint32_t entryPointCount) : pShaderBlob(pBlob)
	{
		stateSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		stateSubobject.pDesc = &dxilLibDesc;

		dxilLibDesc = {};
		exportDesc.resize(entryPointCount);
		exportName.resize(entryPointCount);
		if (pBlob)
		{
			dxilLibDesc.DXILLibrary.pShaderBytecode = pBlob->GetBufferPointer();
			dxilLibDesc.DXILLibrary.BytecodeLength = pBlob->GetBufferSize();
			dxilLibDesc.NumExports = entryPointCount;
			dxilLibDesc.pExports = exportDesc.data();

			for (uint32_t i = 0; i < entryPointCount; i++)
			{
				exportName[i] = entryPoint[i];
				exportDesc[i].Name = exportName[i];
				exportDesc[i].Flags = D3D12_EXPORT_FLAG_NONE;
				exportDesc[i].ExportToRename = nullptr;
			}
		}
	};

	DxilLibrary() : DxilLibrary(nullptr, std::vector<const WCHAR*>(), 0) {}

	D3D12_DXIL_LIBRARY_DESC dxilLibDesc = {};
	D3D12_STATE_SUBOBJECT stateSubobject{};
	ID3DBlobPtr pShaderBlob;
	std::vector<D3D12_EXPORT_DESC> exportDesc;
	std::vector<LPCWSTR> exportName;
};



struct RootSignatureDesc
{
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	std::vector<D3D12_DESCRIPTOR_RANGE> range;
	std::vector<D3D12_ROOT_PARAMETER> rootParams;
};


struct ExportAssociation
{
	ExportAssociation(const WCHAR** exportNames, int count, const D3D12_STATE_SUBOBJECT* pSubobjectToAssociate)
	{
		association.NumExports = count;
		association.pExports = exportNames;
		association.pSubobjectToAssociate = pSubobjectToAssociate;

		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
		subobject.pDesc = &association;
	}

	ExportAssociation(const WCHAR** exportNames, const D3D12_STATE_SUBOBJECT* pSubobjectToAssociate)
	{
		association.NumExports = 1;
		association.pExports = exportNames;
		association.pSubobjectToAssociate = pSubobjectToAssociate;

		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
		subobject.pDesc = &association;
	}

	ExportAssociation()
	{
		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
		subobject.pDesc = &association;
	}

	D3D12_STATE_SUBOBJECT subobject;
	D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION association;
};

struct ShaderConfig
{
	ShaderConfig(uint32_t maxAttributeSizeInBytes, uint32_t maxPayloadSizeInBytes)
	{
		shaderConfig.MaxAttributeSizeInBytes = maxAttributeSizeInBytes;
		shaderConfig.MaxPayloadSizeInBytes = maxPayloadSizeInBytes;

		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		subobject.pDesc = &shaderConfig;
	}

	D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
	D3D12_STATE_SUBOBJECT subobject = {};
};


struct PipelineConfig
{
	PipelineConfig(uint32_t maxTraceRecursionDepth)
	{
		config.MaxTraceRecursionDepth = maxTraceRecursionDepth;

		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		subobject.pDesc = &config;
	}

	D3D12_RAYTRACING_PIPELINE_CONFIG config = {};
	D3D12_STATE_SUBOBJECT subobject = {};
};



#pragma once
class RendererUtil
{
public:

	// Convert a blob to at string
	template<class BlotType>
	static std::string convertBlobToString(BlotType* pBlob)
	{
		std::vector<char> infoLog(pBlob->GetBufferSize() + 1);
		memcpy(infoLog.data(), pBlob->GetBufferPointer(), pBlob->GetBufferSize());
		infoLog[pBlob->GetBufferSize()] = 0;
		return std::string(infoLog.data());
	}

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

	static DxilLibrary CreateDxilLibrary(HWND mWinHandle, std::wstring& shaderFilename, const std::vector<const WCHAR*>& entryPoints);

	static ID3DBlobPtr CompileLibrary(HWND mWinHandle, const WCHAR* filename, const WCHAR* targetString);
	static ID3D12RootSignaturePtr CreateRootSignature(HWND mWinHandle, ID3D12Device5Ptr pDevice, const D3D12_ROOT_SIGNATURE_DESC& desc);







	static RootSignatureDesc CreateRayGenRootDesc();
	static RootSignatureDesc CreateHitRootDesc();
	static RootSignatureDesc CreateHitRootDesc2();






	static ID3D12ResourcePtr CreateConstantBuffer(HWND winHandle, ID3D12Device5Ptr device, void* data, uint32_t bufferSize);

	static void ResourceBarrier(ID3D12GraphicsCommandList4Ptr pCmdList, ID3D12ResourcePtr pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);


	static const D3D12_HEAP_PROPERTIES kUploadHeapProps; 
	static const D3D12_HEAP_PROPERTIES kDefaultHeapProps;

};

struct LocalRootSignature
{
	LocalRootSignature(HWND mWinHandle, ID3D12Device5Ptr pDevice, const D3D12_ROOT_SIGNATURE_DESC& desc)
	{
		pRootSig = RendererUtil::CreateRootSignature(mWinHandle, pDevice, desc);
		pInterface = pRootSig.GetInterfacePtr();
		subobject.pDesc = &pInterface;
		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
	}

	LocalRootSignature()
	{
		pRootSig = nullptr;
	}

	ID3D12RootSignaturePtr pRootSig;
	ID3D12RootSignature* pInterface = nullptr;
	D3D12_STATE_SUBOBJECT subobject = {};
};

struct GlobalRootSignature
{
	GlobalRootSignature(HWND mWinHandle, ID3D12Device5Ptr pDevice, const D3D12_ROOT_SIGNATURE_DESC& desc)
	{
		pRootSig = RendererUtil::CreateRootSignature(mWinHandle, pDevice, desc);
		pInterface = pRootSig.GetInterfacePtr();
		subobject.pDesc = &pInterface;
		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	}
	ID3D12RootSignaturePtr pRootSig;
	ID3D12RootSignature* pInterface = nullptr;
	D3D12_STATE_SUBOBJECT subobject = {};
};

struct Camera
{
	void CreateCamera(HWND winHandle, ID3D12Device5Ptr device);
	void UpdateCamera();

	ID3D12ResourcePtr mCameraBuffer;
	ID3D12DescriptorHeapPtr mConstHeap;
	uint32_t mCameraBufferSize;

	glm::vec3 Eye; 
	glm::vec3 Dir;
	glm::vec3 Up;
};


struct HitProgram
{
	HitProgram(LPCWSTR ahsExport, const WCHAR* chsExport, const std::wstring name, LocalRootSignature* lrs = nullptr) : exportName(name), localRootSignature(lrs)
	{
		desc = {};
		desc.AnyHitShaderImport = nullptr;
		desc.ClosestHitShaderImport = chsExport;
		desc.HitGroupExport = exportName.c_str();

		subObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		subObject.pDesc = &desc;

		exportStr = chsExport;
	}

	std::wstring exportName;
	const WCHAR* exportStr;

	D3D12_HIT_GROUP_DESC desc;
	D3D12_STATE_SUBOBJECT subObject;
	LocalRootSignature* localRootSignature = nullptr;
};

struct MissProgram
{
	MissProgram(const WCHAR* missExport, LocalRootSignature* lrs = nullptr) : missShader(missExport), localRootSignature(lrs)
	{
		
	}

	const WCHAR* missShader;
	LocalRootSignature* localRootSignature = nullptr;
};