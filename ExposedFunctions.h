#pragma once

#include "MyHelper.h"

namespace Exposed {

	typedef bool (*PrePostPatchCallback)(YYTKCodeEvent* , void*);
	std::vector<PrePostPatchCallback> PrePatchCallbacks;
	std::vector<PrePostPatchCallback> PostPatchCallbacks;

HWND GetWindowHandle()
{
	YYRValue yyhwnd = Binds::CallBuiltinA("window_handle", {});
	const char* cchwnd = (const char*)yyhwnd;
	return (HWND)cchwnd;
}

void InstallPrePatch(PrePostPatchCallback function)
{
	PrePatchCallbacks.push_back(function);
}

void InstallPostPatch(PrePostPatchCallback function)
{
	PostPatchCallbacks.push_back(function);
}

int ExposeFunctions(PluginAttributes_t* pa)
{
	int status = YYTK_OK;
	if (PmSetExported(pa, "API_GetWindowHandle", GetWindowHandle) != YYTK_OK)
	{
		Misc::Print("Failed to PmSetExported API_GetWindowHandle()", CLR_RED);
		status = YYTK_FAIL;
	};

	if (PmSetExported(pa, "API_InstallPrePatch", InstallPrePatch) != YYTK_OK)
	{
		Misc::Print("Failed to PmSetExported API_InstallPrePatch()", CLR_RED);
		status = YYTK_FAIL;
	};

	if (PmSetExported(pa, "API_InstallPostPatch", InstallPostPatch) != YYTK_OK)
	{
		Misc::Print("Failed to PmSetExported API_InstallPostPatch()", CLR_RED);
		status = YYTK_FAIL;
	};

	return status;
}

}