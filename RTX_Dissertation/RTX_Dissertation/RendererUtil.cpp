#include "RendererUtil.h"
#include <comdef.h>

void RendererUtil::ShowD3DErrorMessage(HWND hwnd, HRESULT hr)
{
	auto message = _com_error(hr).ErrorMessage();
	std::string msg = wstring_2_string(message);

	char hr_msg[512];
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, hr, 0, hr_msg, ARRAYSIZE(hr_msg), nullptr);

	std::string error_msg = msg + ".\nError! " + hr_msg;
	DisplayMessage(hwnd, error_msg);
}
