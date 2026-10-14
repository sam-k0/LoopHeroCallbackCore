#pragma once

#include "MyPlugin.h"
#include <mutex>

typedef int (*PrePostPatchCallback)(YYTKCodeEvent*, void*);
std::vector<PrePostPatchCallback> PrePatchCallbacks;
std::vector<PrePostPatchCallback> PostPatchCallbacks;

// Mods install patches from their own threads while the game thread is running them, so every access to the lists is locked
std::mutex PatchCallbacksMutex;

// Returns nullptr once index is past the end. The lock isn't held while the callback runs, so callbacks can install patches themselves.
PrePostPatchCallback GetPatchCallback(const std::vector<PrePostPatchCallback>& callbacks, size_t index)
{
    std::lock_guard<std::mutex> lock(PatchCallbacksMutex);
    return index < callbacks.size() ? callbacks[index] : nullptr;
}

void API_InstallPrePatch(PrePostPatchCallback function)
{
    //Misc::Print("Installing PrePatch Method");
    std::lock_guard<std::mutex> lock(PatchCallbacksMutex);
    PrePatchCallbacks.push_back(function);
}

void API_InstallPostPatch(PrePostPatchCallback function)
{
    //Misc::Print("Installing PostPatch Method");
    std::lock_guard<std::mutex> lock(PatchCallbacksMutex);
    PostPatchCallbacks.push_back(function);
}

HWND API_GetWindowHandle()
{
    YYRValue yyhwnd = Binds::CallBuiltinA("window_handle", {});
    return yyhwnd.As<HWND>();
}



