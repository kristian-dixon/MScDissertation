#pragma once
#include "GameSystem.h"
class PointLightSystem :
	public GameSystem
{
public:
	explicit PointLightSystem()
		: GameSystem(static_cast<int>(EComponentType::PointLight))	{}

	//TODO:: Create list of active lights for optimisation.
	void Run() override;
};

