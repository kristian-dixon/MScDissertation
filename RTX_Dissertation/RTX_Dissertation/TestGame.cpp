#include "TestGame.h"
#include "ResourceManager.h"
#include "RendererUtil.h"

void TestGame::OnLoad(HWND winHandle, uint32_t winWidth, uint32_t winHeight)
{

	auto sizeTest = sizeof(glm::vec3);

	SetMouse(winHandle);

	//Initialise renderer
	auto renderer = Renderer::CreateInstance(winHandle, winWidth, winHeight);
	renderer->InitDXR();

	/*auto mesh = ResourceManager::RequestMesh("TRIANGLE");

	mat4 transformMat = mat4();
	mesh->AddInstance(transformMat);

	transformMat = translate(mat4(), vec3(2, 0, 5));
	mesh->AddInstance(transformMat);
	*/
	
	auto mesh = ResourceManager::RequestMesh("CUBE");

	auto transformMat = translate(mat4(), vec3(-10, 5, 10));
	mesh->AddInstance(transformMat);
	/*
	mesh = ResourceManager::RequestMesh("QUAD");
	transformMat = translate(mat4(), vec3(2, 0, 0.25f));
	mesh->AddInstance(transformMat);
	*/

	/*auto mesh = ResourceManager::RequestMesh("SPHERE");
	auto transformMat = translate(mat4(), vec3(-10, -1, 15.25f));
	transformMat = glm::rotate(transformMat, glm::radians(180.f), vec3(1, 1, 0));
	mesh->AddInstance(transformMat);
	*/

	
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
	Sleep(1);
	//TODO::Draw things
	Renderer::GetInstance()->Render();
}

void TestGame::Shutdown()
{
	ResourceManager::ClearResources();

	Renderer::GetInstance()->Shutdown();
}

void TestGame::KeyboardInput(int key)
{
	auto& camera = Renderer::GetInstance()->GetCamera();

	//TODO:: FACTOR IN DT

	if(key == 'A')
	{
		

		//MOVE LEFT
		camera.Eye += glm::cross(mForward, vec3(0, 1, 0)) * (mMovSpeed * -1);
	}
	else if(key == 'D')
	{
		camera.Eye += glm::cross(mForward, vec3(0, 1, 0)) * mMovSpeed;
	}
	else if(key == 'W')
	{
		camera.Eye += mForward * mMovSpeed;
	}
	else if (key == 'S')
	{
		camera.Eye += mForward * (mMovSpeed * -1);
	}
	else if (key == 'R')
	{
		camera.Eye += glm::vec3(0, 1, 0) * mMovSpeed;
	}
	else if (key == 'F')
	{
		camera.Eye += glm::vec3(0, -1, 0) * mMovSpeed;
	}
}

void TestGame::MouseInput()
{
	if (mMouse)
	{
		mMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);

		auto state = mMouse->GetState();
		yaw += state.x * 0.001f;
		pitch += state.y * 0.001f;

	
		auto forward = glm::vec3(0, 0, 1);
		auto up = glm::vec3(0, 1, 0);

		mat4 yawRot = glm::rotate(-yaw, vec3(0, 1, 0));
		mat4 pitchRot = glm::rotate(pitch, vec3(1, 0, 0));

		forward = mat3(pitchRot) * forward;
		forward = mat3(yawRot) * forward;
	


		auto& camera = Renderer::GetInstance()->GetCamera();
		camera.Dir = (forward);

		mForward = forward;
	}
}
