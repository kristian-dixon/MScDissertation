#pragma once
#include "GameComponent.h"
class PointLightComponent :
	public GameComponent
{
public:
	PointLightComponent(GameObject* owner)
		: GameComponent(owner, EComponentType::PointLight)
	{
	}
};

