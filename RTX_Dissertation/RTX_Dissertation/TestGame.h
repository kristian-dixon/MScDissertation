#pragma once
#include "Game.h"


class TestGame :
	public Game
{
public:
	~TestGame() override = default;
	void OnLoad(HWND winHandle, uint32_t winWidth, uint32_t winHeight) override;
	void Update() override;
	void Render() override;
	void Shutdown() override;
};

