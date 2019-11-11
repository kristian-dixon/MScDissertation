#pragma once
#include "Renderer.h"


class Game
{
public:
	virtual ~Game() = default;
	virtual void OnLoad(LPSTR& lpCmdLine, HWND winHandle, uint32_t winWidth, uint32_t winHeight) = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void Shutdown() = 0;

	virtual void KeyDown(int key) = 0;
	virtual void KeyUp(int key) = 0;


	virtual void MouseInput() = 0;
};

