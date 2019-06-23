#include "TestGame.h"

void TestGame::OnLoad(HWND winHandle, uint32_t winWidth, uint32_t winHeight)
{
	//TODO::Load Assets	
	//TODO::Init renderer

	renderer.Init(winHandle, winWidth, winHeight);
}

void TestGame::Update()
{
	//TODO::Update things
	
}

void TestGame::Render()
{
	//TODO::Draw things
	renderer.Render();
}

void TestGame::Shutdown()
{
	renderer.Shutdown();
}
