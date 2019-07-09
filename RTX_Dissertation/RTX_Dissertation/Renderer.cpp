#include "Renderer.h"

Renderer* Renderer::m_instance = nullptr;

Renderer* Renderer::CreateInstance(HWND winHandle, uint32_t winWidth, uint32_t winHeight)
{
	return nullptr;
}

void Renderer::Init(HWND winHandle, uint32_t winWidth, uint32_t winHeight)
{
}

void Renderer::Render()
{
}

void Renderer::Shutdown()
{
}
