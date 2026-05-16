#pragma once

#include "MyPlugin.h"

typedef int (*PrePostPatchCallback)(YYTKCodeEvent*, void*);
std::vector<PrePostPatchCallback> PrePatchCallbacks;
std::vector<PrePostPatchCallback> PostPatchCallbacks;


void API_InstallPrePatch(PrePostPatchCallback function)
{
    //Misc::Print("Installing PrePatch Method");
    PrePatchCallbacks.push_back(function);
}

void API_InstallPostPatch(PrePostPatchCallback function)
{
    //Misc::Print("Installing PostPatch Method");
    PostPatchCallbacks.push_back(function);
}

HWND API_GetWindowHandle()
{
    YYRValue yyhwnd = Binds::CallBuiltinA("window_handle", {});
    const char* cchwnd = (const char*)yyhwnd;
    return (HWND)cchwnd;
}



