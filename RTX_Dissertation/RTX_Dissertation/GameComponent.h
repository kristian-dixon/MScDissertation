#pragma once
#include "GameObject.h"

enum class EComponentType : int
{
	NONE = 0,
	SinusoidalMotion = 1 << 0,
	/*material = 1 << 1,
	physics = 1 << 2,
	sphereCollider = 1 << 3,
	explosion = 1 << 4,
	timer = 1 << 5,
	flightStatus = 1 << 6,
	respawn = 1 << 7,
	lookAt = 1 << 8,
	input = 1 << 9,
	weld = 1 << 10,
	camera = 1 << 11,
	light = 1 << 12,
	orbit = 1 << 13*/
};

class GameComponent
{
	GameComponent(std::shared_ptr<GameObject> owner, EComponentType componentType) : mOwner(owner), mComponentType(componentType){};
	std::shared_ptr<GameObject> GetOwner() const { return mOwner; };
	EComponentType GetComponentType() const { return mComponentType; };

	std::shared_ptr<GameObject> mOwner = nullptr;
	EComponentType mComponentType = EComponentType::NONE;

protected:
};

