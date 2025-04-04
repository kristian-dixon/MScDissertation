#include "TimeManager.h"
#include <chrono>
#include "imgui.h"

TimeManager* TimeManager::instance = nullptr;

TimeManager::TimeManager()
{
	mStartTime = std::chrono::system_clock::now();
	mLastFrameTime = mStartTime;
}

void TimeManager::Update()
{
	auto thisFrameTime = std::chrono::system_clock::now();
	mDT = ImGui::GetIO().DeltaTime;//std::chrono::duration<float>(thisFrameTime - mLastFrameTime).count();
	mLastFrameTime = thisFrameTime;

	mElapsedTime += mDT;
}
