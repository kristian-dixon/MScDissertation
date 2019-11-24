#pragma once

#include "RendererUtil.h"
#include "Mesh.h"
#include <map>
#include "Vertex.h"
#include "ShaderTable.h"
#include <nlohmann/json.hpp>

using namespace glm;

enum class TLASUpdateStyle : int
{
	NONE = 0,
	Refit  = 1,
	Rebuild = 2,
	Mix = 3
};

class Mesh;

class Renderer
{
public:
	static Renderer* CreateInstance(HWND winHandle, uint32_t winWidth, uint32_t winHeight, nlohmann::basic_json<>::value_type& desc);
	static Renderer* GetInstance() { return mInstance; };

	void Render();
	void Shutdown();
	ID3D12ResourcePtr CreateVertexBuffer(const std::vector<Vertex>& verts);
	ID3D12ResourcePtr CreateIndexBuffer(const std::vector<uint32_t>& verts);

	void InitDXR();

	void CreateDXRResources();

	Camera& GetCamera() { return mCamera; };

	HWND GetWindowHandle() { return mWinHandle; };
	ID3D12Device5Ptr GetDevice() { return mpDevice; };

private:
	Renderer(HWND winHandle, uint32_t winWidth, uint32_t winHeight, nlohmann::basic_json<>::value_type& desc);


	struct
	{
		ID3D12CommandAllocatorPtr pCmdAllocator;
		ID3D12ResourcePtr pSwapChainBuffer;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	} mFrameObjects[3];




	AccelerationStructureBuffers CreateBLAS(std::shared_ptr<Mesh> mesh);
	void BuildTLAS(const std::map<std::string, std::shared_ptr<Mesh>>&, uint64_t& tlasSize, bool update, AccelerationStructureBuffers& buffers);
	void CreateAccelerationStructures();

	void CreateShaderResources();
	uint32_t BeginFrame();

	void EndFrame(uint32_t rtvIndex);

	//Properties
private:
	std::wstring mShaderFileName = L"Data/Shaders.hlsl";
	TLASUpdateStyle mTLASUpdateStyle = TLASUpdateStyle::NONE;
	int mRecursionDepth = 5;
	float rebuildFrequency = 1;
	float mLastRebuildTime = 0;

	ID3D12ResourcePtr mTestCbuffer;


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
	uint64_t mFenceValue = 0;


	AccelerationStructureBuffers mTLAS;
	uint64_t mTlasSize = 0;


	ID3D12RootSignaturePtr mpEmptyRootSig;
	ID3D12StateObjectPtr mpPipelineState;

	ID3D12ResourcePtr mpOutputResource;
	ID3D12DescriptorHeapPtr mpSrvUavHeap;

	ID3D12ResourcePtr mpShaderTable;
	ShaderTable mShaderTable;


	Camera mCamera;

	ID3D12ResourcePtr testCB;

	ID3D12ResourcePtr raytraceSettings;

	//CONSTANTS
	static const D3D12_HEAP_PROPERTIES kUploadHeapProps;
	static const D3D12_HEAP_PROPERTIES kDefaultHeapProps;
	static const uint32_t kSrvUavHeapSize = 2;
	static const uint32_t k_RtvHeapSize;
	//static const uint32_t kDefaultSwapChainBuffers ;

};

