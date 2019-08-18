#include "TestGame.h"
#include "ResourceManager.h"
#include "RendererUtil.h"

#include "ConstantBuffers.h"

#include <chrono>
#include <ctime>

void TestGame::OnLoad(HWND winHandle, uint32_t winWidth, uint32_t winHeight)
{

	auto sizeTest = sizeof(glm::vec3);

	SetMouse(winHandle);

	

	//Initialise renderer
	auto renderer = Renderer::CreateInstance(winHandle, winWidth, winHeight);
	renderer->InitDXR();

	LoadHitPrograms();


	{
		const auto hitGroupPointer = ResourceManager::RequestHitProgram("HitGroup");
		const auto shadowHitGroupPointer = ResourceManager::RequestHitProgram("ShadowHitGroup");
		const auto metalHitGroupPointer = ResourceManager::RequestHitProgram("MetalHitGroup");
		const auto rippleHitGroupPointer = ResourceManager::RequestHitProgram("RippleHitGroup");



		MaterialBuffer redMat{ vec3(1,0,0),0, vec3(0,1,0), 0, 10 };

		const auto redMatCB = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &redMat, sizeof(MaterialBuffer));


		MaterialBuffer blueMat{ vec3(0,0,1), 0, vec3(1,1,1), 0, 15 };
		auto blueMatCB = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &blueMat, sizeof(MaterialBuffer));


		MaterialBuffer whiteMat{ vec3(0.8f,0.8f,0.8f),0, vec3(1,1,1),0, 5 };
		auto whiteMatCB = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &whiteMat, sizeof(MaterialBuffer));

		MaterialBuffer funMat{ vec3(-1, -1, -1),0, vec3(1,1,1), 0,8 };
		auto funMatCB = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &funMat, sizeof(MaterialBuffer));

		MetalBuffer metalBuffer{ 0.05f, vec3(0), 0.0f, vec3() };
		auto metalCB = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &metalBuffer, sizeof(MetalBuffer));

		MetalBuffer roughMetalBuffer{ 0.05f, vec3(), 0.05f, vec3() };
		auto roughMetalCB = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &roughMetalBuffer, sizeof(MetalBuffer));

		
		MetalBuffer testMetalBuffer{ 0.85f, vec3(), 0.0f, vec3() };
		auto testMetalCB = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &testMetalBuffer, sizeof(MetalBuffer));

		worldBuffer = { vec3(-0.2, 0.5, -0.5), 0, vec3(2, 1.9f, 1.5f), 0,0 };
		worldCB = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), &worldBuffer, sizeof(WorldBuffer));

		
		vector<ID3D12ResourcePtr> buffers; buffers.push_back(redMatCB); buffers.push_back(worldCB);

		mat4 transformMat = mat4();

		Instance instance = Instance(transformMat, { hitGroupPointer, shadowHitGroupPointer }, buffers);

		auto mesh = ResourceManager::RequestMesh("TRIANGLE");

		mesh->AddInstance(instance);


		mesh = ResourceManager::RequestMesh("CUBE");
		instance.SetTransform(translate(mat4(), vec3(0, -2, 0)));

		mesh->AddInstance(instance);

		mat4 baseTransform = translate(mat4(), vec3(10, 0, 10));
		
		for(int i = 0; i < 5; i++)
		{
			for(int j = 0; j < 5 - i; j++)
			{
				transformMat = translate(baseTransform, vec3(-i, j, i + j) * 2.f);
				instance.SetTransform(transformMat);
				mesh->AddInstance(instance);

				transformMat = translate(baseTransform, vec3(i, j, i + j) * 2.f);
				instance.SetTransform(transformMat);
				mesh->AddInstance(instance);
				

				transformMat = translate(baseTransform, vec3(i, j, -(i + j) + 8) * 2.f);
				instance.SetTransform(transformMat);
				mesh->AddInstance(instance);

				transformMat = translate(baseTransform, vec3(-i, j, -(i + j) + 8) * 2.f);
				instance.SetTransform(transformMat);
				mesh->AddInstance(instance);
			}
		}
		

		buffers.clear();
		buffers.push_back(blueMatCB); buffers.push_back(worldCB);
		transformMat = translate(mat4(), vec3(-2, 0, 3));
		
		instance = Instance(transformMat, { hitGroupPointer, shadowHitGroupPointer }, buffers);
		mesh->AddInstance(instance);


		transformMat = translate(mat4(), vec3(7.95, 0, 9.9));
		instance.SetTransform(transformMat);
		mesh->AddInstance(instance);



		transformMat = translate(mat4(), vec3(12.05, 0, 9.9));
		instance.SetTransform(transformMat);
		mesh->AddInstance(instance);

		
		buffers.clear();
		buffers.push_back(metalCB);
		buffers.push_back(whiteMatCB);
		buffers.push_back(worldCB);
		instance = Instance(transformMat, { rippleHitGroupPointer, shadowHitGroupPointer }, buffers);


		transformMat = translate(mat4(), vec3(0, -1, 0));
		transformMat = scale(transformMat, vec3(100, 0.001, 100));

		instance.SetTransform(transformMat);

		mesh->AddInstance(instance);




		mesh = ResourceManager::RequestMesh("QUAD");
		transformMat = translate(mat4(), vec3(2, 0, 0.25f));
		instance.SetTransform(transformMat);

		//mesh->AddInstance(instance);

		
		mesh = ResourceManager::RequestMesh("SPHERE");
		transformMat = translate(mat4(), vec3(-10, 2, 15.25f));

		
		buffers.clear();
		buffers.push_back(funMatCB);
		buffers.push_back(worldCB);
		
		instance = Instance(transformMat, { hitGroupPointer, shadowHitGroupPointer }, buffers);

		instance.SetTransform(transformMat);
		mesh->AddInstance(instance);
		
		transformMat = translate(mat4(), vec3(-37.5f, 5.f, 0.f));
		instance.SetTransform(transformMat);
		mesh->AddInstance(instance);

		
		
		buffers.clear();
		buffers.push_back(metalCB);
		buffers.push_back(blueMatCB);
		buffers.push_back(worldCB);
		instance = Instance(transformMat, { metalHitGroupPointer, shadowHitGroupPointer }, buffers);

		
		//auto mesh = ResourceManager::RequestMesh("SPHERE");
		for(int x = 0; x < 10; x++)
		{
			for(int z = 0; z < 10; z++)
			{
				transformMat = translate(mat4(), vec3(-125.5f + x * 25, 10, -125.5f + z * 25));
				instance.SetTransform(transformMat);
				mesh->AddInstance(instance);
			}
		}
		
		
		//Infinite mirrors
		mesh = ResourceManager::RequestMesh("CUBE");
		transformMat = translate(mat4(), vec3(-25, 8, 0));
		transformMat = scale(transformMat, vec3(1, 8, 10));

		instance.SetTransform(transformMat);
		mesh->AddInstance(instance);

		transformMat = translate(transformMat, vec3(-25, 0, 0));
		instance.SetTransform(transformMat);
		mesh->AddInstance(instance);




		//Reflectance CB tests
		mesh = ResourceManager::RequestMesh("SPHERE");

		buffers.clear();
		buffers.push_back(roughMetalCB);
		buffers.push_back(redMatCB);
		buffers.push_back(worldCB);
		transformMat = translate(mat4(), vec3(-35, 0, 0));
		instance = Instance(transformMat, { metalHitGroupPointer, shadowHitGroupPointer }, buffers);
		mesh->AddInstance(instance);


		buffers.clear();
		buffers.push_back(testMetalCB);
		buffers.push_back(redMatCB);
		buffers.push_back(worldCB);
		transformMat = translate(mat4(), vec3(-40, 0, 0));
		instance = Instance(transformMat, { metalHitGroupPointer, shadowHitGroupPointer }, buffers);
		mesh->AddInstance(instance);


	}


	//Create final renderer resources
	renderer->CreateDXRResources();

	mLastFrameTime = std::chrono::system_clock::now();
	
}

void TestGame::LoadHitPrograms()
{
	auto renderer = Renderer::GetInstance();

	vector<D3D12_ROOT_PARAMETER> chsRootParams(5);
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
		chsRootParams[3].Descriptor.ShaderRegister = 1;

		chsRootParams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		chsRootParams[4].Descriptor.RegisterSpace = 0;
		chsRootParams[4].Descriptor.ShaderRegister = 3;
	}

	vector<D3D12_ROOT_PARAMETER> metalRootParams(6);
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
		metalRootParams[3].Descriptor.ShaderRegister = 2;

		metalRootParams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		metalRootParams[4].Descriptor.RegisterSpace = 0;
		metalRootParams[4].Descriptor.ShaderRegister = 1;

		metalRootParams[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		metalRootParams[5].Descriptor.RegisterSpace = 0;
		metalRootParams[5].Descriptor.ShaderRegister = 3;
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
