#pragma once
#include <vector>
#include "GameObject.h"

using namespace std;

class GameSystem
{
	int mMask = 0;

protected:
	vector<GameObject*> mGameObjects;

public:
	virtual ~GameSystem() = default;

	GameSystem(int mask) : mMask(mask){};

	void AddGameObject(GameObject* go)
	{
		for (int i = 0; i < mGameObjects.size(); i++)
		{
			if(mGameObjects[i] == go)
			{
				return;
			}
		}
	
		mGameObjects.push_back(go);
	};

	//Warning -- Not yet implemented definitely need to now it's raw
	void RemoveGameObject(shared_ptr<GameObject> go) { /*TODO:: Implement*/ throw std::exception("Not Implemented"); };

	virtual void Run() = 0;

	bool CompareMask(int otherMask) const
	{
		return ((mMask & otherMask) == mMask);
	}
};

