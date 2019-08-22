#pragma once
#include "Game.h"
#include <Mouse.h>
#include <chrono>
#include "ConstantBuffers.h"
#include "PerformanceCapture.h"

class TestGame :
	public Game
{
public:
	~TestGame() override = default;
	void OnLoad(HWND winHandle, uint32_t winWidth, uint32_t winHeight) override;
	void LoadHitPrograms();
	void Update() override;
	void Render() override;
	void Shutdown() override;

	void KeyboardInput(int key) override;

	void SetMouse(HWND winHandle) { mMouse = std::make_unique<DirectX::Mouse>(); mMouse->SetWindow(winHandle); };// mMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);

	void MouseInput() override;

	size_t animationTestHook;
private:
	float mMovSpeed = 0.2f;
	float yaw = 0;
	float pitch = 0;


	std::chrono::system_clock::time_point mLastFrameTime;

	glm::vec3 mForward;
	

	std::unique_ptr<DirectX::Mouse> mMouse;

	WorldBuffer worldBuffer;
	ID3D12ResourcePtr worldCB;

	PerformanceCapture mPerfCapture = PerformanceCapture();
};

