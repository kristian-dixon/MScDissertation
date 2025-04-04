#include <locale>
#include <codecvt>
#include <windows.h>
#include "windowsx.h"
#include <d3d12.h>
#include "TestGame.h"
#include <Mouse.h>
#include "TimeManager.h"
#include "IMGUI_Implementation.h"
#include <filesystem>
#include <iostream>
#include <fstream>

using namespace std;
HWND gWinHandle = nullptr;
TestGame game;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


static LRESULT CALLBACK msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
	{
		return true;
	}

	switch (msg)
	{
	
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) PostQuitMessage(0);
		else
		{
			game.KeyDown(wParam);
		}
		return 0;
	case WM_KEYUP:
		{
			game.KeyUp(wParam);
		}
		return 0;
	case WM_ACTIVATEAPP:
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		DirectX::Mouse::ProcessMessage(msg, wParam, lParam);
		game.MouseInput();
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

static LRESULT CALLBACK msgProc2(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
	{
		return true;
	}

	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}


void msgBox(const std::string& msg)
{
	MessageBoxA(gWinHandle, msg.c_str(), "Error", MB_OK);
}

//From NVIDIA's RTX tutorial
std::wstring string_2_wstring(const std::string& s)
{
	std::wstring_convert<std::codecvt_utf8<WCHAR>> cvt;
	std::wstring ws = cvt.from_bytes(s);
	return ws;
}

HWND OpenWindow(int windowWidth, int windowHeight)
{
	//Create window
	const WCHAR* className = L"WindowClass";
	DWORD winStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

	WNDCLASS wc = {};
	wc.lpfnWndProc = msgProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = className;

	if(RegisterClass(&wc) == 0)
	{
		msgBox("RegisterClass() failed");
		return nullptr;
	}

	RECT r{ 0,0, (LONG)windowWidth, (LONG)windowHeight };
	AdjustWindowRect(&r, winStyle, false);

	int width = r.right - r.left;
	int height = r.bottom - r.top;

	std::wstring wTitle = string_2_wstring("RTX Dissertation");
	HWND hWnd = CreateWindowEx(0, className, wTitle.c_str(), winStyle, 500, 100, width, height, nullptr, nullptr, wc.hInstance, nullptr);

	if(hWnd == nullptr)
	{
		msgBox("CreateWindowEX() has failed");
		return nullptr;
	}

	return hWnd;
}

HWND OpenWindow2(int windowWidth, int windowHeight)
{
	//Create window
	const WCHAR* className = L"SettingsWindow";
	DWORD winStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

	WNDCLASS wc = {};
	wc.lpfnWndProc = msgProc2;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = className;

	if (RegisterClass(&wc) == 0)
	{
		msgBox("RegisterClass() failed");
		return nullptr;
	}

	RECT r{ 0,0, (LONG)windowWidth, (LONG)windowHeight };
	AdjustWindowRect(&r, winStyle, false);

	int width = r.right - r.left;
	int height = r.bottom - r.top;

	std::wstring wTitle = string_2_wstring("RTX Dissertation");
	HWND hWnd = CreateWindowEx(0, className, wTitle.c_str(), winStyle, 0, 0, width, height, nullptr, nullptr, wc.hInstance, nullptr);

	if (hWnd == nullptr)
	{
		msgBox("CreateWindowEX() has failed");
		return nullptr;
	}

	return hWnd;
}

void msgLoop()
{
	auto time = TimeManager::GetInstance();

	MSG msg;
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{

			static bool skip = false;
			
			if(!skip)
			{
				IMGUI_Implementation::Update(msg);
				IMGUI_Implementation::Render();
			}

			skip != skip;
			//DO LOGIC AND RENDERING
			time->Update();


			game.Update();
			game.Render();

		}
	}
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	//Minor TODO:: Make this a part of the config file
	int windowWidth = 1200;
	int windowHeight = 720;

	auto secondaryWindow = OpenWindow2(600, 720);
	//ShowWindow(secondaryWindow, SW_SHOWNORMAL);

	IMGUI_Implementation::CreateIMGUIWindow(secondaryWindow);

	string filePath = "";
	while(filePath == "")
	{
		MSG msg;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//Scene selector
		filePath = IMGUI_Implementation::ShowMainMenu(windowWidth, windowHeight);
		IMGUI_Implementation::Render();
	}

	gWinHandle = OpenWindow(windowWidth, windowHeight);

	RECT r;
	GetClientRect(gWinHandle, &r);
	windowWidth = r.right - r.left;
	windowHeight = r.bottom - r.top;

	//Load things
	game.OnLoad(filePath, gWinHandle, windowWidth, windowHeight);

	//Show window
	ShowWindow(gWinHandle, SW_SHOWNORMAL);

	
	
	//Start message loop
	msgLoop();

	//Cleanup
	game.Shutdown();
	DestroyWindow(gWinHandle);
}
