#pragma once
#include "Renderer.h"


class Game
{
public:
	virtual ~Game() = default;
	virtual void OnLoad(HWND winHandle, uint32_t winWidth, uint32_t winHeight) = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void Shutdown() = 0;

	virtual void KeyboardInput(int key) = 0;

	virtual void MouseInput(float x, float y) = 0;
};

