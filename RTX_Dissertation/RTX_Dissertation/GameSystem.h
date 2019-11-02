#pragma once
#include <vector>
#include "GameObject.h"

using namespace std;

class GameSystem
{
	int mMask = 0;
	std::vector<shared_ptr<GameObject>> gameObjects;

	void AddGameObject(shared_ptr<GameObject> go) { gameObjects.push_back(go); };

	//Warning -- Not yet implemented
	void RemoveGameObject(shared_ptr<GameObject> go) { /*TODO:: Implement*/ throw std::exception("Not Implemented"); };

	virtual void Run() = 0;

	bool CheckMask(int otherMask) const
	{
		return ((mMask & otherMask) == mMask);
	}
};

