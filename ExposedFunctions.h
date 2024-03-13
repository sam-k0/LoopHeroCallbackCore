#pragma once

#include "MyHelper.h"

namespace Exposed {


HWND GetWindowHandle()
{
	YYRValue yyhwnd = Binds::CallBuiltinA("window_handle", {});
	const char* cchwnd = (const char*)yyhwnd;
	return (HWND)cchwnd;
}



int ExposeFunctions(PluginAttributes_t* pa)
{
	int status = YYTK_OK;
	if (PmSetExported(pa, "API_GetWindowHandle", GetWindowHandle) != YYTK_OK)
	{
		Misc::Print("Failed to PmSetExported API_GetWindowHandle()", CLR_RED);
		status = YYTK_FAIL;
	};

	return status;
}

}