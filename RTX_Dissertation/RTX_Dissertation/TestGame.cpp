#include "TestGame.h"
#include "ResourceManager.h"
#include "RendererUtil.h"

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

		const auto pinkGroupPointer = ResourceManager::RequestHitProgram("GridGroup");


		void* redPtr = new vec4(1, 0, 0, 1);

		const auto cb = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), redPtr, sizeof(vec4));
		vector<ID3D12ResourcePtr> buffers; buffers.push_back(cb);

		mat4 transformMat = mat4();

		Instance instance = Instance(transformMat, { hitGroupPointer, shadowHitGroupPointer }, buffers);
		//Instance instance = Instance(transformMat, { pinkGroupPointer }, vector<ID3D12ResourcePtr>());


		auto mesh = ResourceManager::RequestMesh("TRIANGLE");

		mesh->AddInstance(instance);


		mesh = ResourceManager::RequestMesh("CUBE");
		instance.SetTransform(translate(mat4(), vec3(0, -2, 0)));

		//mesh->AddInstance(instance);

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
		
		void* bluePtr = new vec4(0, 0, 1, 1);
		auto cblue = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), bluePtr, sizeof(vec4));

		buffers.clear();
		buffers.push_back(cblue);
		transformMat = translate(mat4(), vec3(-2, 0, 3));
		
		instance = Instance(transformMat, { hitGroupPointer, shadowHitGroupPointer }, buffers);
		mesh->AddInstance(instance);


		transformMat = translate(mat4(), vec3(0, 0, 10));
		instance.SetTransform(transformMat);

		mesh->AddInstance(instance);

		void* whitePtr = new vec4(1, 1, 1, 1);
		auto cwhite = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), whitePtr, sizeof(vec4));

		buffers.clear();
		buffers.push_back(cwhite);
		instance = Instance(transformMat, { hitGroupPointer, shadowHitGroupPointer }, buffers);

		transformMat = translate(mat4(), vec3(0, -1, 0));
		transformMat = scale(transformMat, vec3(100, 0.001, 100));

		instance.SetTransform(transformMat);

		mesh->AddInstance(instance);
		
		
		mesh = ResourceManager::RequestMesh("QUAD");
		transformMat = translate(mat4(), vec3(2, 0, 0.25f));
		instance.SetTransform(transformMat);

		mesh->AddInstance(instance);

		
		mesh = ResourceManager::RequestMesh("SPHERE");
		transformMat = translate(mat4(), vec3(-10, 2, 15.25f));

		void* funColour = new vec4(-1, -1, -1, -1);
		auto cFun = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), funColour, sizeof(vec4));

		buffers.clear();
		buffers.push_back(cFun);
		instance = Instance(transformMat, { hitGroupPointer, shadowHitGroupPointer }, buffers);

		instance.SetTransform(transformMat);
		mesh->AddInstance(instance);
		
		
		/*
		instance = Instance(transformMat, { pinkGroupPointer }, vector<ID3D12ResourcePtr>());

		
		//auto mesh = ResourceManager::RequestMesh("SPHERE");
		for(int x = 0; x < 10; x++)
		{
			for(int z = 0; z < 10; z++)
			{
				transformMat = translate(mat4(), vec3(-125.5f + x * 25, 5, -125.5f + z * 25));
				instance.SetTransform(transformMat);
				animationTestHook = mesh->AddInstance(instance);
			}
		}
		
		


		transformMat = translate(mat4(), vec3(8, 3, 14.25));
		
		instance.SetTransform(transformMat);
		
		mesh->AddInstance(instance);*/
	}


	//Create final renderer resources
	renderer->CreateDXRResources();

	mLastFrameTime = std::chrono::system_clock::now();

}

void TestGame::LoadHitPrograms()
{
	auto renderer = Renderer::GetInstance();
	LocalRootSignature rgsRootSignature(renderer->GetWindowHandle(), renderer->GetDevice(), RendererUtil::CreateRayGenRootDesc().desc);

	ResourceManager::AddHitProgram("HitGroup", make_shared<HitProgram>(nullptr, L"chs", L"HitGroup", &rgsRootSignature));
	ResourceManager::AddHitProgram("GridGroup", make_shared<HitProgram>(nullptr, L"grid", L"GridGroup", nullptr));
	ResourceManager::AddHitProgram("ShadowHitGroup", make_shared<HitProgram>(nullptr, L"shadowChs", L"ShadowHitGroup"));

}

void TestGame::Update()
{


	auto thisFrameTime = std::chrono::system_clock::now();
	float dt = chrono::duration<float>(thisFrameTime - mLastFrameTime).count();

	SetWindowTextA(Renderer::GetInstance()->GetWindowHandle(), to_string(1.f / dt).c_str());

	mLastFrameTime = std::chrono::system_clock::now();
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
