#pragma once
#include <vector>
#include "GameObject.h"

using namespace std;

class GameSystem
{
	int mMask = 0;
	std::vector<shared_ptr<GameObject>> gameObjects;


public:
	void AddGameObject(shared_ptr<GameObject> go)
	{
		for (int i = 0; i < gameObjects.size(); i++)
		{
			if(gameObjects[i] == go)
			{
				return;
			}
		}
	
		gameObjects.push_back(go);
	};

	//Warning -- Not yet implemented
	void RemoveGameObject(shared_ptr<GameObject> go) { /*TODO:: Implement*/ throw std::exception("Not Implemented"); };

	virtual void Run() = 0;

	bool CompareMask(int otherMask) const
	{
		return ((mMask & otherMask) == mMask);
	}
};

