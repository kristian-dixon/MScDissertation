#pragma once

#include "RendererUtil.h"
#include "Mesh.h"
#include <map>

using namespace glm;

class Mesh;

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
	ID3D12Resource* CreateVertexBuffer(const std::vector<vec3>& verts);

private:
	Renderer(HWND winHandle, uint32_t winWidth, uint32_t winHeight);
	void InitDXR();


	struct
	{
		ID3D12CommandAllocatorPtr pCmdAllocator;
		ID3D12ResourcePtr pSwapChainBuffer;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	} mFrameObjects[3];



	AccelerationStructureBuffers CreateBLAS(std::shared_ptr<Mesh> mesh);
	void BuildTLAS(const std::map<std::string, std::shared_ptr<Mesh>>&, uint64_t& tlasSize, bool update, AccelerationStructureBuffers& buffers);
	void CreateAccelerationStructures();

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
	uint64_t mFenceValue = 0;


	AccelerationStructureBuffers mTopLevelBuffers;
	uint64_t mTlasSize = 0;


	//CONSTANTS
	static const D3D12_HEAP_PROPERTIES kUploadHeapProps;
	static const D3D12_HEAP_PROPERTIES kDefaultHeapProps;
	static const uint32_t k_RtvHeapSize;
	//static const uint32_t kDefaultSwapChainBuffers ;

};

