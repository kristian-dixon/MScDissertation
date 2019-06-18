#include <locale>
#include <codecvt>
#include <windows.h>
#include <d3d12.h>
#include "TestGame.h"

HWND gWinHandle = nullptr;


static LRESULT CALLBACK msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
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
	HWND hWnd = CreateWindowEx(0, className, wTitle.c_str(), winStyle, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, wc.hInstance, nullptr);

	if(hWnd == nullptr)
	{
		msgBox("CreateWindowEX() has failed");
		return nullptr;
	}

	return hWnd;
}

void msgLoop()
{


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
			//DO LOGIC AND RENDERING
		}
	}
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	TestGame game;

	int windowWidth = 1920;
	int windowHeight = 1200;

	gWinHandle = OpenWindow(windowWidth, windowHeight);

	RECT r;
	GetClientRect(gWinHandle, &r);
	windowWidth = r.right - r.left;
	windowHeight = r.bottom - r.top;

	//Load things
	game.OnLoad(gWinHandle, windowWidth, windowHeight);

	//Show window
	ShowWindow(gWinHandle, SW_SHOWNORMAL);

	

	//Start message loop
	msgLoop();

	//Cleanup

	DestroyWindow(gWinHandle);
}
