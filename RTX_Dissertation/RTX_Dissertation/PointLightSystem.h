#pragma once
#include "GameSystem.h"
#include "ConstantBuffers.h"

class PointLightSystem :
	public GameSystem
{
public:
	explicit PointLightSystem(WorldBuffer& worldBuffer)
		: GameSystem(static_cast<int>(EComponentType::PointLight)), mWorldBuffer(worldBuffer)	{}

	//TODO:: Create list of active lights for optimisation.
	void Run() override;

private:
	WorldBuffer& mWorldBuffer;
};

