#pragma once

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
//#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include <vector>
#include <functional>

class IMGUI_Implementation
{
public:
	static std::vector< std::function<bool()>> imguiObjects;


	static void CreateIMGUIWindow(HWND hwnd);

	static void Update(MSG msg);

	static void Render();

	static void Cleanup();


	

};

