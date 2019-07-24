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

	void KeyboardInput(int key) override;

	void MouseInput(float x, float y) override;

private:
	glm::vec3 lastMousePos;
	float mMovSpeed = 0.2f;
};

