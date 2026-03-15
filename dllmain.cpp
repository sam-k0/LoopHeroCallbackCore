// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>

// YYTK is in this now
#include "MyHelper.h"
#include "ModuleManager.h"
#include "LHObjects.h"
#include "LHSprites.h"
#include "Assets.h"
#include "ExposedFunctions.h"
// Plugin functionality
#include <fstream>
#include <iterator>
#include <map>>
#define _CRT_SECURE_NO_WARNINGS


void ShowWelcomeMessage()
{
    Misc::Print("Press F12 to list registered mods.", CLR_GOLD);
}
// Unload
YYTKStatus PluginUnload()
{
    PmRemoveCallback(callbackAttr);

    return YYTK_OK;
}

YYTKStatus ExecuteCodeCallback(YYTKCodeEvent* codeEvent, void*)
{
    CCode* codeObj = std::get<CCode*>(codeEvent->Arguments());
    CInstance* selfInst = std::get<0>(codeEvent->Arguments());
    CInstance* otherInst = std::get<1>(codeEvent->Arguments());
    // If we have invalid data???
    if (!codeObj)
        return YYTK_INVALIDARG;

    if (!codeObj->i_pName)
        return YYTK_INVALIDARG;

    // call registered patches
    for (PrePostPatchCallback ThisPrePatch : PrePatchCallbacks)
    {
        ThisPrePatch(codeEvent, nullptr);
    }

    // Original event
    codeEvent->Call(selfInst, otherInst, codeObj, std::get<3>(codeEvent->Arguments()), std::get<4>(codeEvent->Arguments()));
   
    for (PrePostPatchCallback ThisPostPatch : PostPatchCallbacks)
    {
        ThisPostPatch(codeEvent, nullptr);
    }

    return YYTK_OK;
}


// Entry
DllExport YYTKStatus PluginEntry(
    YYTKPlugin* PluginObject // A pointer to the dedicated plugin object
)
{   
    ShowWelcomeMessage();
    while (GetYYTKModule() == nullptr)
    {
        // waiting for yytk
    }

    //Misc::Print("Exporting function...", CLR_YELLOW);
    gThisPlugin = PluginObject;
    gThisPlugin->PluginUnload = PluginUnload;

    PluginAttributes_t* pluginAttributes = nullptr;
    if (PmGetPluginAttributes(gThisPlugin, pluginAttributes) == YYTK_OK)
    {
        PmCreateCallback(pluginAttributes, callbackAttr, reinterpret_cast<FNEventHandler>(ExecuteCodeCallback), EVT_CODE_EXECUTE, nullptr);
    }

    gReady = true;
    
    // Set exported "ready" fn
    PluginAttributes_t* pAttr = nullptr;
    if (PmGetPluginAttributes(PluginObject, pAttr) != YYTK_OK)
    {
        Misc::Print("Failed to PmGetPluginAttributes", CLR_RED);
        return YYTK_FAIL;
    }

    

    // Export RegisterModule and UnregisterModule
    if (PmSetExported(pAttr, "RegisterModule", RegisterModule) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported RegisterModule()", CLR_RED);
        return YYTK_FAIL;
    };

    if (PmSetExported(pAttr, "UnregisterModule", UnregisterModule) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported UnregisterModule()", CLR_RED);
        return YYTK_FAIL;
    };

    if (PmSetExported(pAttr, "API_InstallPostPatch", InstallPostPatch) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported API_InstallPostPatch()", CLR_RED);
        return YYTK_FAIL;
    };

    if (PmSetExported(pAttr, "API_GetWindowHandle", GetWindowHandle) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported API_GetWindowHandle()", CLR_RED);
        return YYTK_FAIL;
    };

    if (PmSetExported(pAttr, "API_InstallPrePatch", InstallPrePatch) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported API_InstallPrePatch()", CLR_RED);
        return YYTK_FAIL;
    };

    if (PmSetExported(pAttr, "CoreReady", Ready) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported Ready()", CLR_RED);
        return YYTK_FAIL;
    };

    Misc::PrintDbg("Exported functions correctly. Mods can load these now.", (__FUNCTION__), __LINE__, CLR_GOLD);

    return YYTK_OK; // Successful PluginEntry.
}


// Export raw pointers using dllexport
DllExport int rPmGetExported(const char* szRoutineName, void*& pfnOutRoutine)
{
	return PmGetExported(szRoutineName, pfnOutRoutine);
}



DWORD WINAPI KeyControls(HINSTANCE hModule)
{
    while(true)
    {
        if (GetAsyncKeyState(VK_F12))
        {   
            if(!gReady)continue;

            // List all registered mods
            PrintRegisteredMods();            
        }
        Sleep(100);
    }
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Start a thread to listen for button presses
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeyControls, NULL, 0, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

