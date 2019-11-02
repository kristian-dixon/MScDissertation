#pragma once
#include <vector>
#include <memory>
#include "GameSystem.h"
#include <queue>

using namespace std;

class SystemManager
{
	static shared_ptr<SystemManager> mInstance;

	vector<shared_ptr<GameSystem>> mSystems;
	queue<GameObject*> mDirtyGameobjects;

public:
	static auto GetInstance()
	{
		if(mInstance == nullptr)
		{
			mInstance = std::make_shared<SystemManager>();
		}

		return mInstance;
	}

	void AddSystem(shared_ptr<GameSystem> val) { mSystems.push_back(val); };

	//TODO:: Add system removal

	void AddGameObjectToUpdateQueue(GameObject* go) { mDirtyGameobjects.push(go); };
	void UpdateDirtyGameObjects();

	void Update();
};

