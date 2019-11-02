#include "SystemManager.h"

shared_ptr<SystemManager> SystemManager::mInstance = nullptr;

void SystemManager::UpdateDirtyGameObjects()
{

	while(!mDirtyGameobjects.empty())
	{
		auto currentGameObject = mDirtyGameobjects.front();
		mDirtyGameobjects.pop();

		for(auto& system : mSystems)
		{
			if(system->CompareMask(currentGameObject->GetComponentMask()))
			{
				//Attempt to add if not already
				system->AddGameObject(currentGameObject);
			}
			else
			{
				//Possibly conisider removing if need be -- Maybe have a previous mask so you know if it previously existed in here
				
			}
		}
	}
}

void SystemManager::Update()
{
	UpdateDirtyGameObjects();

	for(auto& system : mSystems)
	{
		system->Run();
	}
}