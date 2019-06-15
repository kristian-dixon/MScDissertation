#pragma once
#include "Game.h"
class TestGame :
	public Game
{
public:
	~TestGame() override = default;
	void OnLoad() override;
	void Update() override;
	void Render() override;
};

