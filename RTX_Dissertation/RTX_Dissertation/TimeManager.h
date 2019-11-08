#pragma once
#include <chrono>
#include <ctime>

using namespace std;
using namespace  chrono;

class TimeManager
{
	static TimeManager* instance;// = nullptr;

	TimeManager();
	
	system_clock::time_point mLastFrameTime;
	system_clock::time_point mStartTime;
	float mDT = 0.016f;

public:
	static TimeManager* GetInstance()
	{
		if(instance == nullptr)
		{
			instance = new TimeManager;
		}

		return instance;
	}

	void Update();

	auto GetDT() const { return mDT; };
};

