#pragma once
class Game
{
public:
	virtual ~Game() = default;
	virtual void OnLoad();
	virtual void Update();
	virtual void Render();
};

