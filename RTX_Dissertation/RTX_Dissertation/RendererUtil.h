#include <string>

#include <Windows.h>
#include <codecvt>


#pragma once
class RendererUtil
{
public:
	static std::wstring string_2_wstring(const std::string& s)
	{
		std::wstring_convert<std::codecvt_utf8<WCHAR>> cvt;
		std::wstring ws = cvt.from_bytes(s);
		return ws;
	}
	static std::string wstring_2_string(const std::wstring& ws)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
		std::string s = cvt.to_bytes(ws);
		return s;
	}
	static void ShowD3DErrorMessage(HWND hwnd, HRESULT hr);
	static void DisplayMessage(HWND winHandle, const std::string& msg, const std::string& type = "Error") { MessageBoxA(winHandle, msg.c_str(), type.c_str(), MB_OK); }
	static void D3DCall(HWND winHandle, HRESULT hr) { if (FAILED(hr)) { RendererUtil::ShowD3DErrorMessage(winHandle, hr); } }

	

};

