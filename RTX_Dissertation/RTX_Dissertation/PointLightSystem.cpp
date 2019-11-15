#include "PointLightSystem.h"

void PointLightSystem::Run()
{
	//TODO:: CHECK IF LIGHTS ARE ACTIVE AND OPTIMISATION
	//For now just the first 5 lights added are active
	for(int i = 0; i < mGameObjects.size() && i < 5; i++)
	{
		mWorldBuffer.pointLights[i].position = mGameObjects[i]->GetCurrentTransform()[3];
	}

	mWorldBuffer.pointLightCount = mGameObjects.size() < 5 ? mGameObjects.size() : 5;
}
