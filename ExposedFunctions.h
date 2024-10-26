#pragma once

#include "MyHelper.h"

typedef int (*PrePostPatchCallback)(YYTKCodeEvent*, void*);
std::vector<PrePostPatchCallback> PrePatchCallbacks;
std::vector<PrePostPatchCallback> PostPatchCallbacks;


void InstallPrePatch(PrePostPatchCallback function)
{
    //Misc::Print("Installing PrePatch Method");
    PrePatchCallbacks.push_back(function);
}

void InstallPostPatch(PrePostPatchCallback function)
{
    //Misc::Print("Installing PostPatch Method");
    PostPatchCallbacks.push_back(function);
}

HWND GetWindowHandle()
{
    YYRValue yyhwnd = Binds::CallBuiltinA("window_handle", {});
    const char* cchwnd = (const char*)yyhwnd;
    return (HWND)cchwnd;
}



