#include "TestGame.h"
#include "ResourceManager.h"

void TestGame::OnLoad(HWND winHandle, uint32_t winWidth, uint32_t winHeight)
{
	//Initialise renderer
	auto renderer = Renderer::CreateInstance(winHandle, winWidth, winHeight);
	renderer->InitDXR();

	auto mesh = ResourceManager::RequestMesh("TRIANGLE");

	mat4 transformMat = mat4();
	mesh->AddInstance(transformMat);

	/*transformMat = translate(mat4(), vec3(2, 0, 0));
	mesh->AddInstance(transformMat);
	*/



	//Create final renderer resources
	renderer->CreateDXRResources();
}

void TestGame::Update()
{
	//TODO::Update things
	
}

void TestGame::Render()
{
	//TODO::Draw things
	Renderer::GetInstance()->Render();
}

void TestGame::Shutdown()
{
	Renderer::GetInstance()->Shutdown();
}
