#include "Renderer.h"

Renderer* Renderer::m_instance = nullptr;

Renderer* Renderer::CreateInstance(HWND winHandle, uint32_t winWidth, uint32_t winHeight)
{
	if (m_instance)
	{
		//Might not be a good idea but we need to have a way to say "DON'T TRY TO CREATE MULTIPLE RENDERERS"
		throw;
	}
	else
	{
		m_instance = new Renderer(winHandle, winWidth, winHeight);
	}
	return m_instance;
}



Renderer::Renderer(HWND winHandle, uint32_t winWidth, uint32_t winHeight) : mWinHandle(winHandle), mSwapChainSize(winWidth, winHeight)
{
}

void Renderer::InitDXR()
{
	//Initialize the debug layer
	#ifdef _DEBUG

	ID3D12DebugPtr pDebug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug))))
	{
		pDebug->EnableDebugLayer();
	}
	
	#endif 

	// Create the DXGI factory
	IDXGIFactory4Ptr pDxgiFactory;
	D3DCall(CreateDXGIFactory1(IID_PPV_ARGS(&pDxgiFactory)));
	/*auto mpDevice = createDevice(pDxgiFactory);
	auto mpCmdQueue = createCommandQueue(mpDevice);
	auto mpSwapChain = createDxgiSwapChain(pDxgiFactory, mHwnd, winWidth, winHeight, DXGI_FORMAT_R8G8B8A8_UNORM, mpCmdQueue);
	*/
}

void Renderer::Render()
{
}

void Renderer::Shutdown()
{
}
