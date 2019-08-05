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

	LoadHitPrograms();


	{
		const auto hitGroupPointer = ResourceManager::RequestHitProgram("HitGroup");
		const auto pinkGroupPointer = ResourceManager::RequestHitProgram("GridGroup");

		const auto shadowHitGroupPointer = ResourceManager::RequestHitProgram("ShadowHitGroup");

		void* redPtr = new vec4(1, 0, 0, 1);

		const auto cb = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), redPtr, sizeof(vec4));
		vector<ID3D12ResourcePtr> buffers; buffers.push_back(cb);

		mat4 transformMat = mat4();

		Instance instance = Instance(transformMat, { hitGroupPointer, shadowHitGroupPointer }, buffers);
	

		auto mesh = ResourceManager::RequestMesh("TRIANGLE");

		mesh->AddInstance(instance);


		
		transformMat = translate(mat4(), vec3(2, 5, 5));
		instance.SetTransform(transformMat);

		mesh->AddInstance(instance);


		void* bluePtr = new vec4(0, 0, 1, 1);
		auto cblue = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), bluePtr, sizeof(vec4));

		buffers.clear();
		buffers.push_back(cblue);
		instance = Instance(transformMat, { hitGroupPointer, shadowHitGroupPointer }, buffers);


		mesh = ResourceManager::RequestMesh("CUBE");
	    

		transformMat = translate(mat4(), vec3(0, 0, 10));
		instance.SetTransform(transformMat);

		mesh->AddInstance(instance);

		void* whitePtr = new vec4(1, 1, 1, 1);
		auto cwhite = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), whitePtr, sizeof(vec4));

		buffers.clear();
		buffers.push_back(cwhite);
		instance = Instance(transformMat, { hitGroupPointer, shadowHitGroupPointer }, buffers);


		transformMat = translate(mat4(), vec3(0, -5, 0));
		transformMat = scale(transformMat, vec3(100, 1, 100));


		instance.SetTransform(transformMat);

		mesh->AddInstance(instance);
		

		mesh = ResourceManager::RequestMesh("QUAD");
		transformMat = translate(mat4(), vec3(2, 0, 0.25f));
		instance.SetTransform(transformMat);

		mesh->AddInstance(instance);

		
		mesh = ResourceManager::RequestMesh("SPHERE");
		transformMat = translate(mat4(), vec3(-10, -1, 15.25f));

		void* funColour = new vec4(-1, -1, -1, -1);
		auto cFun = RendererUtil::CreateConstantBuffer(Renderer::GetInstance()->GetWindowHandle(), Renderer::GetInstance()->GetDevice(), funColour, sizeof(vec4));

		buffers.clear();
		buffers.push_back(cFun);
		instance = Instance(transformMat, { hitGroupPointer, shadowHitGroupPointer }, buffers);



		instance.SetTransform(transformMat);

		//transformMat = glm::rotate(transformMat, glm::radians(180.f), vec3(1, 1, 0));
		mesh->AddInstance(instance);
		///*
		

		instance = Instance(transformMat, { pinkGroupPointer, shadowHitGroupPointer }, vector<ID3D12ResourcePtr>());

		transformMat = translate(mat4(), vec3(10, 5, 10));
		instance.SetTransform(transformMat);

		mesh->AddInstance(instance);

		//transformMat = translate(mat4(), vec3(8, 3, 14.25));
		//
		//instance.SetTransform(transformMat);
		//
		//mesh->AddInstance(instance);
	}


	//Create final renderer resources
	renderer->CreateDXRResources();
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
