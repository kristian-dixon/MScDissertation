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

	transformMat = translate(mat4(), vec3(2, 0, 0));
	mesh->AddInstance(transformMat);
	
	transformMat = translate(mat4(), vec3(2, 0, 5));
	mesh->AddInstance(transformMat);

	transformMat = translate(mat4(), vec3(-4, 0, 3));
	transformMat = glm::rotate(transformMat, 1.f, vec3(0, 0, 1));
	mesh->AddInstance(transformMat);

	mesh = ResourceManager::RequestMesh("CUBE");

	transformMat = translate(mat4(), vec3(-10, 5, 10));
	mesh->AddInstance(transformMat);


	
	/*
	mesh = ResourceManager::RequestMesh("SPHERE");
	transformMat = translate(mat4(), vec3(-10, -5, 10));

	mesh->AddInstance(transformMat);*/

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
