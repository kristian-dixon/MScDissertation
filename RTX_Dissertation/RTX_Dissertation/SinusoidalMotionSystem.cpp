#include "SinusoidalMotionSystem.h"
#include "SinusoidalMotionComponent.h"
#include "TimeManager.h"

void SinusoidalMotionSystem::Run()
{
	elapsedTime += TimeManager::GetInstance()->GetDT(); //TODO:: Write time manager
	
	for(auto& gameObject : mGameObjects)
	{
		auto originalTransform = gameObject->GetOriginalTransform();

		auto comp = gameObject->GetComponent(EComponentType::SinusoidalMotion);
		auto smc = static_cast<SinusoidalMotionComponent*>(comp.get());

		auto& data = smc->GetData();

		float x = (sin((elapsedTime + data.xSinDtOffset) * data.xSinFreq) * data.xSinAmplitude) + (cos((elapsedTime + data.xCosDtOffset) * data.xCosFreq) * data.xCosAmplitude);
		float y = (sin((elapsedTime + data.ySinDtOffset) * data.ySinFreq) * data.ySinAmplitude) + (cos((elapsedTime + data.yCosDtOffset) * data.yCosFreq) * data.yCosAmplitude);
		float z = (sin((elapsedTime + data.zSinDtOffset) * data.zSinFreq) * data.zSinAmplitude) + (cos((elapsedTime + data.zCosDtOffset) * data.zCosFreq) * data.zCosAmplitude);

		gameObject->SetTransform( translate(originalTransform, vec3(x, y, z)));

	}
}
