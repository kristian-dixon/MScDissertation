#pragma once
#include "Renderer.h"


class Game
{
public:
	virtual ~Game() = default;
	virtual void OnLoad(HWND winHandle, uint32_t winWidth, uint32_t winHeight);
	virtual void Update();
	virtual void Render();
};

