#include "RendererUtil.h"
#include <comdef.h>
#include "DirectXMath.h"

const D3D12_HEAP_PROPERTIES RendererUtil::kUploadHeapProps =
{
	D3D12_HEAP_TYPE_UPLOAD,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0,
};


const D3D12_HEAP_PROPERTIES RendererUtil::kDefaultHeapProps =
{
	D3D12_HEAP_TYPE_DEFAULT,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0
};

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


DxilLibrary RendererUtil::CreateDxilLibrary(HWND mWinHandle, std::wstring& shaderFilename, const std::vector<const WCHAR*>& entryPoints)
{
	// Compile the shader
	ID3DBlobPtr pDxilLib = CompileLibrary(mWinHandle, shaderFilename.c_str(), L"lib_6_3");
	return DxilLibrary(pDxilLib, entryPoints, static_cast<uint32_t>(entryPoints.size()));
}

ID3DBlobPtr RendererUtil::CompileLibrary(HWND winHandle, const WCHAR* filename, const WCHAR* targetString)
{
	// Initialize the helper
	D3DCall(winHandle, gDxcDllHelper.Initialize());
	IDxcCompilerPtr pCompiler;
	IDxcLibraryPtr pLibrary;
	D3DCall(winHandle, gDxcDllHelper.CreateInstance(CLSID_DxcCompiler, &pCompiler));
	D3DCall(winHandle, gDxcDllHelper.CreateInstance(CLSID_DxcLibrary, &pLibrary));

	// Open and read the file
	std::ifstream shaderFile(filename);
	if (shaderFile.good() == false)
	{
		DisplayMessage(winHandle, "Can't open file " + wstring_2_string(std::wstring(filename)));
		return nullptr;
	}
	std::stringstream strStream;
	strStream << shaderFile.rdbuf();
	std::string shader = strStream.str();

	// Create blob from the string
	IDxcBlobEncodingPtr pTextBlob;
	D3DCall(winHandle, pLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)shader.c_str(), (uint32_t)shader.size(), 0, &pTextBlob));

	// Compile
	IDxcOperationResultPtr pResult;
	D3DCall(winHandle, pCompiler->Compile(pTextBlob, filename, L"", targetString, nullptr, 0, nullptr, 0, nullptr, &pResult));

	// Verify the result
	HRESULT resultCode;
	D3DCall(winHandle, pResult->GetStatus(&resultCode));
	if (FAILED(resultCode))
	{
		IDxcBlobEncodingPtr pError;
		D3DCall(winHandle, pResult->GetErrorBuffer(&pError));
		std::string log = convertBlobToString(pError.GetInterfacePtr());
		DisplayMessage(winHandle, "Compiler error:\n" + log);
		return nullptr;
	}

	MAKE_SMART_COM_PTR(IDxcBlob);
	IDxcBlobPtr pBlob;
	D3DCall(winHandle,pResult->GetResult(&pBlob));
	return pBlob;
}


ID3D12RootSignaturePtr RendererUtil::CreateRootSignature(HWND mWinHandle, ID3D12Device5Ptr pDevice, const D3D12_ROOT_SIGNATURE_DESC& desc)
{
	ID3DBlobPtr pSigBlob;
	ID3DBlobPtr pErrorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &pSigBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		std::string msg = convertBlobToString(pErrorBlob.GetInterfacePtr());
		DisplayMessage(mWinHandle, msg);
		return nullptr;
	}

	ID3D12RootSignaturePtr pRootSig;
	D3DCall(mWinHandle,pDevice->CreateRootSignature(0, pSigBlob->GetBufferPointer(), pSigBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSig)));
	return pRootSig;
}

RootSignatureDesc RendererUtil::CreateRayGenRootDesc()
{
	// Create the root-signature
	RootSignatureDesc desc;
	desc.range.resize(4);
	// gOutput
	desc.range[0].BaseShaderRegister = 0;
	desc.range[0].NumDescriptors = 1;
	desc.range[0].RegisterSpace = 0;
	desc.range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	desc.range[0].OffsetInDescriptorsFromTableStart = 0;

	// gRtScene
	desc.range[1].BaseShaderRegister = 0;
	desc.range[1].NumDescriptors = 1;
	desc.range[1].RegisterSpace = 0;
	desc.range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	desc.range[1].OffsetInDescriptorsFromTableStart = 1;

	//CBV
	desc.range[2].BaseShaderRegister = 0;
	desc.range[2].NumDescriptors = 1;
	desc.range[2].RegisterSpace = 0;
	desc.range[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	desc.range[2].OffsetInDescriptorsFromTableStart = 2;

	//CBV
	desc.range[3].BaseShaderRegister = 5;
	desc.range[3].NumDescriptors = 1;
	desc.range[3].RegisterSpace = 0;
	desc.range[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	desc.range[3].OffsetInDescriptorsFromTableStart = 3;

	desc.rootParams.resize(1);
	desc.rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	desc.rootParams[0].DescriptorTable.NumDescriptorRanges = 4;
	desc.rootParams[0].DescriptorTable.pDescriptorRanges = desc.range.data();

	// Create the desc
	desc.desc.NumParameters = 1;
	desc.desc.pParameters = desc.rootParams.data();
	desc.desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

	//{0 /*b0*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_CBV /*Camera parameters*/, 2}});


	return desc;
}

RootSignatureDesc RendererUtil::CreateHitRootDesc(std::vector<D3D12_ROOT_PARAMETER> params)
{
	RootSignatureDesc desc;
	desc.rootParams = params;

	
	desc.desc.NumParameters = static_cast<int>(params.size());
	desc.desc.pParameters = desc.rootParams.data();
	desc.desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

	return desc;
}



ID3D12ResourcePtr RendererUtil::CreateConstantBuffer(HWND winHandle, ID3D12Device5Ptr device, void* data, uint32_t bufferSize)
{
	ID3D12ResourcePtr result = CreateBuffer(winHandle, device, bufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, kUploadHeapProps);
	uint8_t* pData;
	D3DCall(winHandle, result->Map(0, nullptr, (void**)& pData));
	memcpy(pData, data, bufferSize);
	result->Unmap(0, nullptr);
	return result;
}

void RendererUtil::UpdateConstantBuffer(ID3D12ResourcePtr buffer, void* data, uint32_t bufferSize)
{
	uint8_t* pData;
	(buffer->Map(0, nullptr, (void**)& pData));
	memcpy(pData, data, bufferSize);
	buffer->Unmap(0, nullptr);

}


void RendererUtil::ResourceBarrier(ID3D12GraphicsCommandList4Ptr pCmdList, ID3D12ResourcePtr pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = pResource;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = stateBefore;
	barrier.Transition.StateAfter = stateAfter;
	pCmdList->ResourceBarrier(1, &barrier);
}

void Camera::CreateCamera(HWND winHandle, ID3D12Device5Ptr device)
{
	Eye = glm::vec3(0, 2, 0); //DirectX::XMVectorSet(0.0f, 0.0f, -10.5f, 0.0f);
	Dir = glm::vec3(0, 0, 1);//DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	Up = glm::vec3(0, 1, 0);// DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	uint32_t nbMatrix = 4; // view, perspective, viewInv, perspectiveInv
	mCameraBufferSize = nbMatrix * sizeof(DirectX::XMMATRIX);

	// Create the constant buffer for all matrices
	mCameraBuffer = RendererUtil::CreateBuffer(winHandle, device, mCameraBufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, RendererUtil::kUploadHeapProps);

	// Create a descriptor heap that will be used by the rasterization shaders
	mConstHeap = RendererUtil::CreateDescriptorHeap(winHandle, device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

	// Describe and create the constant buffer view.
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = mCameraBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = mCameraBufferSize;

	// Get a handle to the heap memory on the CPU side, to be able to write the descriptors directly
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = mConstHeap->GetCPUDescriptorHandleForHeapStart();
	device->CreateConstantBufferView(&cbvDesc, srvHandle);
}
void Camera::UpdateCamera()
{

	std::vector<DirectX::XMMATRIX > matrices(4);

	// Initialize the view matrix, ideally this should be based on user interactions
	// The lookat and perspective matrices used for rasterization are defined to transform world-space
	// vertices into a [0,1]x[0,1]x[0,1] camera space

	const auto focusPosition = Dir + Eye;

	const auto DXEye = DirectX::XMVectorSet(Eye.x, Eye.y, Eye.z, 0.0f);
	const auto DXAt = DirectX::XMVectorSet(focusPosition.x, focusPosition.y, focusPosition.z, 0.0f);
	const auto DXUp = DirectX::XMVectorSet(Up.x, Up.y, Up.z, 0.0f);


	matrices[0] = DirectX::XMMatrixLookAtRH(DXEye, DXAt, DXUp);

	float fovAngleY = 45.0f * DirectX::XM_PI / 180.0f;
	matrices[1] = DirectX::XMMatrixPerspectiveFovRH(fovAngleY, 1.77f, 0.1f, 1000.0f);

	// Raytracing has to do the contrary of rasterization: rays are defined in camera space, and are
	// transformed into world space. To do this, we need to store the inverse matrices as well.
	DirectX::XMVECTOR det;
	matrices[2] = XMMatrixInverse(&det, matrices[0]);
	matrices[3] = XMMatrixInverse(&det, matrices[1]);

	// Copy the matrix contents
	uint8_t* pData;
	(mCameraBuffer->Map(0, nullptr, (void**)& pData));
	memcpy(pData, matrices.data(), mCameraBufferSize);
	mCameraBuffer->Unmap(0, nullptr);
}
