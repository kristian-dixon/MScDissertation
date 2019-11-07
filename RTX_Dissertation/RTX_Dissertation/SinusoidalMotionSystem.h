#pragma once
#include "GameSystem.h"
class SinusoidalMotionSystem :
	public GameSystem
{
public:
	SinusoidalMotionSystem() : GameSystem(static_cast<int>(EComponentType::SinusoidalMotion)) {};
	void Run() override;
private:

	float elapsedTime = 0;//Temp;
};

