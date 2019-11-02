#pragma once
#include "GameComponent.h"
#include <memory>

struct SinusoidalMotionData
{
	float xSinDtOffset;
	float xSinFreq;
	float xSinAmplitude;
	float xCosDtOffset;
	float xCosFreq;
	float xCosAmplitude;

	float ySinDtOffset;
	float ySinFreq;
	float ySinAmplitude;
	float yCosDtOffset;
	float yCosFreq;
	float yCosAmplitude;

	float zSinDtOffset;
	float zSinFreq;
	float zSinAmplitude;
	float zCosDtOffset;
	float zCosFreq;
	float zCosAmplitude;
};

class SinusoidalMotionComponent :
	public GameComponent
{
public:
	SinusoidalMotionComponent(std::shared_ptr<GameObject> owner, SinusoidalMotionData& data) : GameComponent(owner, EComponentType::SinusoidalMotion), mData(data) {};

	auto& GetData() { return mData };

private:
	SinusoidalMotionData mData;



};

