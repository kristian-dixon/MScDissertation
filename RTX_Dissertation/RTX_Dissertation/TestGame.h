#pragma once
#include "Game.h"
#include <Mouse.h>
#include <chrono>
#include "ConstantBuffers.h"
#include "PerformanceCapture.h"
#include "SystemManager.h"

class TestGame :
	public Game
{
public:
	~TestGame() override = default;
	void OnLoad(string& filePath, HWND winHandle, uint32_t winWidth, uint32_t winHeight) override;
	void LoadShaderPrograms();
	void Update() override;
	void Render() override;
	void Shutdown() override;

	void KeyDown(int key) override;
	void KeyUp(int key) override;
	void ChangeCamera();

	void SetMouse(HWND winHandle) { mMouse = std::make_unique<DirectX::Mouse>(); mMouse->SetWindow(winHandle); };// mMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);

	void MouseInput() override;

	size_t animationTestHook;
private:
	vector<tuple<glm::vec3, glm::vec3>> mCameras;

	int mCamSelection = -1;

	float mMovSpeed = 1.2f;
	float cameraYaw = 0;
	float cameraPitch = 0;

	float sunYaw = 0;
	float sunPitch = 0;


	int zCamVel = 0;
	int xCamVel = 0;

	string mMissShaderName;
	wstring mMissShaderWide;

	std::chrono::system_clock::time_point mLastFrameTime;

	glm::vec3 mForward = vec3(0,0,1);
	glm::vec3 mUp = vec3(0, 1, 0);

	glm::vec3 mSunDir;

	std::unique_ptr<DirectX::Mouse> mMouse;

	WorldBuffer worldBuffer;
	ID3D12ResourcePtr worldCB;

	PerformanceCapture mPerfCapture = PerformanceCapture();

	shared_ptr<SystemManager> mSystemManager = nullptr;

	vector<shared_ptr<GameObject>> gameObjects;
};

