#pragma once

#include "RendererUtil.h"
#include "Mesh.h"
#include <map>

using namespace glm;

class Mesh;

class Renderer
{
public:
	static Renderer* CreateInstance(HWND winHandle, uint32_t winWidth, uint32_t winHeight);
	static Renderer* GetInstance() { return mInstance; };

	void Render();
	void Shutdown();
	ID3D12ResourcePtr CreateVertexBuffer(const std::vector<vec3>& verts);
	ID3D12ResourcePtr CreateIndexBuffer(const std::vector<uint32_t>& verts);

	void InitDXR();

	void CreateDXRResources();

	float x = 0;
private:
	Renderer(HWND winHandle, uint32_t winWidth, uint32_t winHeight);


	struct
	{
		ID3D12CommandAllocatorPtr pCmdAllocator;
		ID3D12ResourcePtr pSwapChainBuffer;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	} mFrameObjects[3];




	AccelerationStructureBuffers CreateBLAS(std::shared_ptr<Mesh> mesh);
	void BuildTLAS(const std::map<std::string, std::shared_ptr<Mesh>>&, uint64_t& tlasSize, bool update, AccelerationStructureBuffers& buffers);
	void CreateAccelerationStructures();

	void CreateRTPipelineState();
	void CreateShaderResources();
	void CreateShaderTable();
	uint32_t BeginFrame();

	void EndFrame(uint32_t rtvIndex);

	//Properties
private:
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
	uint32_t mShaderTableEntrySize = 0;


	Camera mCamera;
	


	//CONSTANTS
	static const D3D12_HEAP_PROPERTIES kUploadHeapProps;
	static const D3D12_HEAP_PROPERTIES kDefaultHeapProps;
	static const uint32_t kSrvUavHeapSize = 2;
	static const uint32_t k_RtvHeapSize;
	//static const uint32_t kDefaultSwapChainBuffers ;

};

