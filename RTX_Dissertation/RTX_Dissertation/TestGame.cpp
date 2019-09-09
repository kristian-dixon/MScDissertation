#include "TestGame.h"
#include "ResourceManager.h"
#include "RendererUtil.h"

#include "ConstantBuffers.h"

#include <chrono>
#include <ctime>

#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>

#include "GameObject.h"
#include "ObjLoader.h"

using json = nlohmann::json;

void TestGame::OnLoad(HWND winHandle, uint32_t winWidth, uint32_t winHeight)
{
	//Initialise renderer
	auto renderer = Renderer::CreateInstance(winHandle, winWidth, winHeight);
	renderer->InitDXR();

	LoadHitPrograms();
	worldBuffer = { vec3(-0.2, 0.5, -0.5), 0, vec3(2, 1.9f, 1.5f), 0,0 };
	worldCB = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &worldBuffer, sizeof(WorldBuffer));

	json fileReadTest;
	fileReadTest << std::ifstream("Props.json");


	//1018-487
	GameObject test;

	auto goList = fileReadTest["GameObjects"];

	for (int i = 0; i < goList.size(); i++)
	{
		test.LoadFromJson(goList[i], worldCB);
	}

	//Create final renderer resources
	renderer->CreateDXRResources();

	mLastFrameTime = std::chrono::system_clock::now();

	SetMouse(winHandle);

}

void TestGame::LoadHitPrograms()
{
	auto renderer = Renderer::GetInstance();

	vector<D3D12_ROOT_PARAMETER> chsRootParams(6);
	{
		chsRootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		chsRootParams[0].Descriptor.RegisterSpace = 0;
		chsRootParams[0].Descriptor.ShaderRegister = 1;

		chsRootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		chsRootParams[1].Descriptor.RegisterSpace = 0;
		chsRootParams[1].Descriptor.ShaderRegister = 2;

		chsRootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		chsRootParams[2].Descriptor.RegisterSpace = 0;
		chsRootParams[2].Descriptor.ShaderRegister = 0;

		chsRootParams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		chsRootParams[3].Descriptor.RegisterSpace = 0;
		chsRootParams[3].Descriptor.ShaderRegister = 4;

		chsRootParams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		chsRootParams[4].Descriptor.RegisterSpace = 0;
		chsRootParams[4].Descriptor.ShaderRegister = 1;

		chsRootParams[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		chsRootParams[5].Descriptor.RegisterSpace = 0;
		chsRootParams[5].Descriptor.ShaderRegister = 3;


		
	}

	vector<D3D12_ROOT_PARAMETER> metalRootParams(7);
	{
		metalRootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		metalRootParams[0].Descriptor.RegisterSpace = 0;
		metalRootParams[0].Descriptor.ShaderRegister = 1;

		metalRootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		metalRootParams[1].Descriptor.RegisterSpace = 0;
		metalRootParams[1].Descriptor.ShaderRegister = 2;

		metalRootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		metalRootParams[2].Descriptor.RegisterSpace = 0;
		metalRootParams[2].Descriptor.ShaderRegister = 0;

		metalRootParams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		metalRootParams[3].Descriptor.RegisterSpace = 0;
		metalRootParams[3].Descriptor.ShaderRegister = 4;

		metalRootParams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		metalRootParams[4].Descriptor.RegisterSpace = 0;
		metalRootParams[4].Descriptor.ShaderRegister = 2;

		metalRootParams[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		metalRootParams[5].Descriptor.RegisterSpace = 0;
		metalRootParams[5].Descriptor.ShaderRegister = 1;

		metalRootParams[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		metalRootParams[6].Descriptor.RegisterSpace = 0;
		metalRootParams[6].Descriptor.ShaderRegister = 3;

		
	}


	LocalRootSignature* rgsRootSignature2 = new LocalRootSignature(renderer->GetWindowHandle(), renderer->GetDevice(), RendererUtil::CreateHitRootDesc(metalRootParams).desc);

	LocalRootSignature* rgsRootSignature = new LocalRootSignature(renderer->GetWindowHandle(), renderer->GetDevice(), RendererUtil::CreateHitRootDesc(chsRootParams).desc);

	ResourceManager::AddHitProgram("MetalHitGroup", make_shared<HitProgram>(nullptr, L"metal", L"MetalHitGroup", rgsRootSignature2));
	ResourceManager::AddHitProgram("RippleHitGroup", make_shared<HitProgram>(nullptr, L"rippleSurface", L"RippleHitGroup", rgsRootSignature2));

	ResourceManager::AddHitProgram("HitGroup", make_shared<HitProgram>(nullptr, L"chs", L"HitGroup", rgsRootSignature));

	ResourceManager::AddHitProgram("GridGroup", make_shared<HitProgram>(nullptr, L"grid", L"GridGroup", nullptr));
	ResourceManager::AddHitProgram("ShadowHitGroup", make_shared<HitProgram>(nullptr, L"shadowChs", L"ShadowHitGroup"));


}

void TestGame::Update()
{
	auto thisFrameTime = std::chrono::system_clock::now();
	float dt = chrono::duration<float>(thisFrameTime - mLastFrameTime).count();


	auto& camera = Renderer::GetInstance()->GetCamera();
	//Update Camera
	camera.Eye += (glm::cross(mForward, vec3(0, xCamVel, 0)) + mForward * (float)zCamVel) * mMovSpeed * dt;



	//mPerfCapture.Update(dt);


	SetWindowTextA(Renderer::GetInstance()->GetWindowHandle(), to_string(1.f / dt).c_str());

	mLastFrameTime = std::chrono::system_clock::now();


	worldBuffer.time += dt;


	RendererUtil::UpdateConstantBuffer(worldCB, &worldBuffer, sizeof(WorldBuffer));





	/*
	//TODO::Update things
	shitTimer += 1 / 60.f;

	auto mesh = ResourceManager::RequestMesh("SPHERE");

	auto animPos = translate(mat4(), vec3(10, 5 + sin(1 * shitTimer) * 10, 10));

	mesh->GetInstances()[animationTestHook].SetTransform(animPos);*/
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

void TestGame::KeyDown(int key)
{
	auto& camera = Renderer::GetInstance()->GetCamera();

	//TODO:: FACTOR IN DT

	if(key == 'A')
	{
		xCamVel = -1;
	}
	else if (key == 'D')
	{
		xCamVel = 1;
	}

	if (key == 'W')
	{
		zCamVel = 1;
	}
	else if (key == 'S')
	{
		zCamVel = -1;
	}

	/*if(key == 'A')
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
	*/
	if(key == 37 || key == 38 || key == 39 || key == 40)
	{
		if(key == 37 || key == 39)
		{
			sunYaw += key == 37 ? 0.01 : -0.01;
		}
		else
		{
			sunPitch += key == 38 ? 0.01 : -0.01;
		}

		//Sun positioning
		auto forward = vec3(-0.2, 0.5, -0.5);
		auto up = glm::vec3(0, 1, 0);

		mat4 yawRot = glm::rotate(-sunYaw, vec3(0, 1, 0));
		mat4 pitchRot = glm::rotate(sunPitch, vec3(1, 0, 0));

		forward = mat3(pitchRot) * forward;
		forward = mat3(yawRot) * forward;
		worldBuffer.sunDir = forward;
	}


	//Toggle Framerate capture (period)
	if (key == 190)
	{
		mPerfCapture.ToggleRecording();
	}

}

void TestGame::KeyUp(int key)
{
	if (key == 'A' || key == 'D')
	{
		xCamVel = 0;
	}
	else if (key == 'W' || key == 'S')
	{
		zCamVel = 0;
	}
}


void TestGame::MouseInput()
{
	if (mMouse)
	{
		mMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);

		auto state = mMouse->GetState();
		cameraYaw += state.x * 0.001f;
		cameraPitch += state.y * 0.001f;

	
		auto forward = glm::vec3(0, 0, 1);
		auto up = glm::vec3(0, 1, 0);

		mat4 yawRot = glm::rotate(-cameraYaw, vec3(0, 1, 0));
		mat4 pitchRot = glm::rotate(cameraPitch, vec3(1, 0, 0));

		forward = mat3(pitchRot) * forward;
		forward = mat3(yawRot) * forward;
	


		auto& camera = Renderer::GetInstance()->GetCamera();
		camera.Dir = (forward);

		mForward = forward;

		mMovSpeed = state.scrollWheelValue * 0.0005 + 1.2f;
	}
}
