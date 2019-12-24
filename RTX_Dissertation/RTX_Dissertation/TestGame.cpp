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
#include "SinusoidalMotionSystem.h"
#include "TimeManager.h"
#include "PointLightSystem.h"

using json = nlohmann::json;

void TestGame::OnLoad(string& filePath, HWND winHandle, uint32_t winWidth, uint32_t winHeight)
{
	json sceneInfo;
	auto f = std::ifstream(filePath);// >> fileReadTest;
	if (f.good())
	{
		f >> sceneInfo;
	}
	else
	{
		//Props should always exist as a error thingy
		std::ifstream("Data/Scenes/Error_Scene.json") >> sceneInfo;
	}

	json renderingSettings;
	std::ifstream("Data/Scenes/" + sceneInfo.value<string>("RendererSettingsFilename", "Error_RenderingSettings.json")) >> renderingSettings;

	json props;
	std::ifstream("Data/Scenes/" + sceneInfo.value<string>("PropsFilepath", "Error_Props.json")) >> props;

	mMissShaderName = renderingSettings.value("MissShaderName", "miss");
	mMissShaderWide = RendererUtil::string_2_wstring(mMissShaderName);
	//Initialise renderer
	auto renderer = Renderer::CreateInstance(winHandle, winWidth, winHeight, renderingSettings);
	renderer->InitDXR();

	worldBuffer = { vec3(-0.2, 0.5, -0.5), 0, vec3(2, 1.9f, 1.5f), 0,0 };
	worldCB = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &worldBuffer, sizeof(WorldBuffer));



	LoadShaderPrograms();

	mSystemManager = SystemManager::GetInstance();
	mSystemManager->AddSystem(std::make_shared<SinusoidalMotionSystem>());
	mSystemManager->AddSystem(std::make_shared<PointLightSystem>(worldBuffer));
	//Load systems
	

	
	auto goList = props["GameObjects"];

	for (int i = 0; i < goList.size(); i++)
	{
		gameObjects.push_back(make_shared<GameObject>());
		gameObjects[i]->LoadFromJson(goList[i], worldCB);
	}

	auto camList = props["Cameras"];
	for (int i = 0; i < camList.size(); i++)
	{
		auto pos = camList[i]["Position"];
		glm::vec3 camPos = glm::vec3(pos["x"], pos["y"], pos["z"]);


		auto fwd = camList[i]["Forward"];
		glm::vec3 camFwd = glm::vec3(fwd["x"], fwd["y"], fwd["z"]);

		mCameras.push_back(make_tuple(camPos, camFwd));
	}

	//Create final renderer resources
	renderer->CreateDXRResources();

	mLastFrameTime = std::chrono::system_clock::now();

	SetMouse(winHandle);
	mMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
}

void TestGame::LoadShaderPrograms()
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


	vector<D3D12_ROOT_PARAMETER> intRootParams(3);
	{
		

		intRootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		intRootParams[0].Descriptor.RegisterSpace = 0;
		intRootParams[0].Descriptor.ShaderRegister = 0;


		intRootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		intRootParams[1].Descriptor.RegisterSpace = 0;
		intRootParams[1].Descriptor.ShaderRegister = 4;


		intRootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		intRootParams[2].Descriptor.RegisterSpace = 0;
		intRootParams[2].Descriptor.ShaderRegister = 3;


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

	vector<D3D12_ROOT_PARAMETER> missRootParams(1);
	{
		missRootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		missRootParams[0].Descriptor.RegisterSpace = 0;
		missRootParams[0].Descriptor.ShaderRegister = 3;
	}

	LocalRootSignature* defaultMatRGS = new LocalRootSignature(renderer->GetWindowHandle(), renderer->GetDevice(), RendererUtil::CreateHitRootDesc(chsRootParams).desc);
	LocalRootSignature* reflectiveRGS = new LocalRootSignature(renderer->GetWindowHandle(), renderer->GetDevice(), RendererUtil::CreateHitRootDesc(metalRootParams).desc);
	LocalRootSignature* timeBasicRGS = new LocalRootSignature(renderer->GetWindowHandle(), renderer->GetDevice(), RendererUtil::CreateHitRootDesc(missRootParams).desc);
	LocalRootSignature* intRGS = new LocalRootSignature(renderer->GetWindowHandle(), renderer->GetDevice(), RendererUtil::CreateHitRootDesc(intRootParams).desc);


	ResourceManager::AddHitProgram("MetalHitGroup", make_shared<HitProgram>(nullptr, L"metal", L"MetalHitGroup", reflectiveRGS));
	ResourceManager::AddHitProgram("RippleHitGroup", make_shared<HitProgram>(nullptr, L"rippleSurface", L"RippleHitGroup", reflectiveRGS));
	ResourceManager::AddHitProgram("TranslucentHitGroup", make_shared<HitProgram>(nullptr, L"translucent", L"TranslucentHitGroup", reflectiveRGS));
	ResourceManager::AddHitProgram("RippleTranslucentHitGroup", make_shared<HitProgram>(nullptr, L"rippleTranslucent", L"RippleTranslucentHitGroup", reflectiveRGS));



	ResourceManager::AddHitProgram("HitGroup", make_shared<HitProgram>(nullptr, L"chs", L"HitGroup", defaultMatRGS));
	ResourceManager::AddHitProgram("TerrainHitGroup", make_shared<HitProgram>(nullptr, L"TerrainCHS", L"TerrainHitGroup", defaultMatRGS));

	ResourceManager::AddHitProgram("LambertianHitGroup", make_shared<HitProgram>(nullptr, L"lambertian", L"LambertianHitGroup", defaultMatRGS));
	ResourceManager::AddHitProgram("EmissiveHitGroup", make_shared<HitProgram>(nullptr, L"emissive", L"EmissiveHitGroup", defaultMatRGS));
	ResourceManager::AddHitProgram("LambertianPointLightHitGroup", make_shared<HitProgram>(nullptr, L"lambertianPointLighting", L"LambertianPointLightHitGroup", defaultMatRGS));
	ResourceManager::AddHitProgram("LambertianHeavyHitGroup", make_shared<HitProgram>(nullptr, L"lambertianHeavy", L"LambertianHeavyHitGroup", defaultMatRGS));


	//
	ResourceManager::AddHitProgram("AO_OneShotGroup", make_shared<HitProgram>(nullptr, L"AO_OneShot", L"AO_OneShotGroup", defaultMatRGS));
	ResourceManager::AddHitProgram("AO_OneShot_IgnoreCHSGroup", make_shared<HitProgram>(nullptr, L"AO_OneShot_IgnoreCHS", L"AO_OneShot_IgnoreCHSGroup", defaultMatRGS));


	ResourceManager::AddHitProgram("GridGroup", make_shared<HitProgram>(nullptr, L"grid", L"GridGroup", defaultMatRGS));
	ResourceManager::AddHitProgram("GridPuddle", make_shared<HitProgram>(nullptr, L"gridPuddle", L"GridPuddleHitGroup", reflectiveRGS));

	ResourceManager::AddHitProgram("ShadowHitGroup", make_shared<HitProgram>(nullptr, L"shadowChs", L"ShadowHitGroup"));

	ResourceManager::AddHitProgram("SphereIntersectHitGroup", make_shared<HitProgram>(L"SphereIntersect", L"SphereClosestHit", L"SphereIntersectHitGroup", intRGS));
	ResourceManager::AddHitProgram("FogIntersectHitGroup", make_shared<HitProgram>(L"VolumetricFogIntersection", L"VolumetricClosestHit", L"FogIntersectHitGroup", intRGS));


	//ResourceManager::AddHitProgram("DirectionalWithGI", make_shared<HitProgram>(nullptr, L"DirectionalWithGI", L"DirectionalWithGI", reflectiveRGS));


	//AO_OneShot


	vector<ID3D12ResourcePtr> missData; missData.push_back(worldCB);
	ResourceManager::AddMissProgram("miss", make_shared<MissProgram>(mMissShaderWide.c_str(), timeBasicRGS, missData));
	


	ResourceManager::AddMissProgram("shadowMiss", make_shared<MissProgram>(L"shadowMiss"/*, timeBasicRGS, missData*/));
}

bool pauseFrame = false;
void TestGame::Update()
{
	
	float dt = TimeManager::GetInstance()->GetDT();

	mSystemManager->Update();
	
	//Update Camera
	auto& camera = Renderer::GetInstance()->GetCamera();
	camera.Eye += (glm::cross(mForward, vec3(0, xCamVel, 0)) + mForward * (float)zCamVel) * mMovSpeed * dt;

	worldBuffer.time += dt;

	RendererUtil::UpdateConstantBuffer(worldCB, &worldBuffer, sizeof(WorldBuffer));
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
			sunYaw += key == 37 ? 0.01f : -0.01f;
		}
		else
		{
			sunPitch += key == 38 ? 0.01f : -0.01f;
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

	if(key == 'U')
	{
		if(mMouse->GetState().positionMode == DirectX::Mouse::MODE_RELATIVE)
		{
			mMouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);
			mMouse->SetVisible(true);
		}
		else
		{
			mMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
		//	mMouse->SetVisible(false);
		}
	}

	if(key == 187)
	{
		mCamSelection = (mCamSelection + 1) % mCameras.size();
		ChangeCamera();
	}

	if(key == 189)
	{
		mCamSelection = (mCamSelection - 1) % mCameras.size();
		ChangeCamera();
	}
}

void TestGame::ChangeCamera()
{
	if (mCameras.empty())return;

	auto& camera = Renderer::GetInstance()->GetCamera();
	camera.Eye = get<0>(mCameras[mCamSelection]);
	mForward = get<1>(mCameras[mCamSelection]);

	camera.Dir = mForward;
}

void TestGame::MouseInput()
{
	if (mMouse)
	{
		//if (mMouse->GetState().positionMode == DirectX::Mouse::MODE_ABSOLUTE) { return; }
		
		auto state = mMouse->GetState();
		cameraYaw = state.x * 0.001f * state.positionMode;
		cameraPitch = -state.y * 0.001f * state.positionMode;

	
		auto forward = mForward;

		auto right = glm::cross(forward, vec3(0, 1, 0));

		mat4 yawRot = glm::rotate(-cameraYaw, vec3(0, 1, 0));
		mat4 pitchRot = glm::rotate(cameraPitch, right);

		forward = mat3(pitchRot) * forward;
		forward = mat3(yawRot) * forward;
	


		auto& camera = Renderer::GetInstance()->GetCamera();
		camera.Dir = (forward);

		mForward = forward;

		mMovSpeed = state.scrollWheelValue * 0.0005f + 1.2f;
	}
}
