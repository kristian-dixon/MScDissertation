#pragma once
#include "GameSystem.h"
class SinusoidalMotionSystem :
	public GameSystem
{
	SinusoidalMotionSystem() : GameSystem(static_cast<int>(EComponentType::SinusoidalMotion)) {};

	void Run() override;
};

