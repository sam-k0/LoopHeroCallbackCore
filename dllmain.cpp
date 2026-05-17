// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>

// YYTK is in this now
#include "MyPlugin.h"
#include "ModuleManager.h"
#include "LHObjects.h"
#include "LHSprites.h"
#include "Assets.h"
#include "CallbackApi.h"
#include "CallbackCoreAttribs.h"
#include "HttpUtil.h"
// Plugin functionality
#include <fstream>
#include <iterator>
#include <map>
#include <format>
#include <optional>
#define _CRT_SECURE_NO_WARNINGS

#define BUTTON_PRESS_EVENT "gml_Object_o_base_button_Mouse_4"
#define BUTTON_ALARM_EVENT "gml_Object_o_menu_button_Alarm_6"

double gModsButtonRef = -1.;

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

    if (!selfInst)
		return YYTK_INVALIDARG;

    // call registered patches
    bool callOriginal = true;
    CallbackCoreAttributes attribs = CallbackCoreAttributes(OriginalCall::EARLY); // Prepatch doesn't know if the original will be called.

    for (PrePostPatchCallback ThisPrePatch : PrePatchCallbacks)
    {
        if (ThisPrePatch(codeEvent, (void*)&attribs) == YYTK_DONTCALL)
        {
            callOriginal = false;
        }
    }
    
    attribs.call = OriginalCall::CALLED; // Default to Called

    // Catch custom HttpEvents and cancel original call
    if (strcmp(codeObj->i_pName, HTTP_EVENT_ID) == 0)
    {
        YYRValue asyncLoadMap = Binds::CallBuiltinA("variable_instance_get", { selfInst, "async_load" });
        YYRValue value = Binds::CallBuiltinA("ds_map_find_value", { asyncLoadMap, "id" });
        // check if it exists in the list of registered custom http events
        if(HttpRequests::IsCustomHttpEvent((double)value))
        {
            callOriginal = false;
            HttpRequests::HandleHttpEvent(asyncLoadMap);
        }
    }

    if (strcmp(codeObj->i_pName, BUTTON_PRESS_EVENT) == 0 && int(gModsButtonRef) == selfInst->i_id)
    {
        Binds::CallBuiltinA("url_open", {"https://github.com/sam-k0/LoopHero_Mods/blob/master/mods.md"});
	}

    if (strcmp(codeObj->i_pName, BUTTON_ALARM_EVENT) == 0 && int(gModsButtonRef) == selfInst->i_id)
    {
		callOriginal = false; // Dont call lang change alarm
    }


    // Original event
    if (callOriginal)
    {
        codeEvent->Call(selfInst, otherInst, codeObj, std::get<3>(codeEvent->Arguments()), std::get<4>(codeEvent->Arguments()));
    }
    else
    {
        codeEvent->Cancel(true);
        attribs.call = OriginalCall::CANCELLED;
    }
   
    for (PrePostPatchCallback ThisPostPatch : PostPatchCallbacks)
    {
        ThisPostPatch(codeEvent, (void*)&attribs);
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
        //PmCreateCallback(pluginAttributes, callbackAttr, reinterpret_cast<FNEventHandler>(ExecuteCodeCallback), EVT_CODE_EXECUTE, nullptr);
        PmCreateCallbackEx(pluginAttributes, 9999, reinterpret_cast<FNEventHandler>(ExecuteCodeCallback), EVT_CODE_EXECUTE, nullptr, callbackAttr);
    }

    gModuleManagerReady = true;
#pragma region ExportFunctions
    // Set exported "ready" fn
    PluginAttributes_t* pAttr = nullptr;
    if (PmGetPluginAttributes(PluginObject, pAttr) != YYTK_OK)
    {
        Misc::Print("Failed to PmGetPluginAttributes", CLR_RED);
        return YYTK_FAIL;
    }

	// Export shared functions for mods to use, such as registering themselves and installing patches
    if (PmSetExported(pAttr, "RegisterModule", API_RegisterModule) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported RegisterModule()", CLR_RED);
        return YYTK_FAIL;
    };

    if (PmSetExported(pAttr, "UnregisterModule", API_UnregisterModule) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported UnregisterModule()", CLR_RED);
        return YYTK_FAIL;
    };

    if (PmSetExported(pAttr, "CoreReady", API_ModuleManagerReadyCheck) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported Ready()", CLR_RED);
        return YYTK_FAIL;
    };

    if (PmSetExported(pAttr, "API_InstallPostPatch", API_InstallPostPatch) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported API_InstallPostPatch()", CLR_RED);
        return YYTK_FAIL;
    };

    if (PmSetExported(pAttr, "API_GetWindowHandle", API_GetWindowHandle) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported API_GetWindowHandle()", CLR_RED);
        return YYTK_FAIL;
    };

    if (PmSetExported(pAttr, "API_InstallPrePatch", API_InstallPrePatch) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported API_InstallPrePatch()", CLR_RED);
        return YYTK_FAIL;
    };
    // Plugin Enumeration API
    if (PmSetExported(pAttr, "API_GetRegisteredPluginCount", API_GetRegisteredPluginCount) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported API_GetRegisteredPluginCount()", CLR_RED);
        return YYTK_FAIL;
    };

    if (PmSetExported(pAttr, "API_GetRegisteredPluginName", API_GetRegisteredPluginName) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported API_GetRegisteredPluginName()", CLR_RED);
        return YYTK_FAIL;
    };

    if (PmSetExported(pAttr, "API_GetRegisteredPluginPresent", API_GetRegisteredPluginPresent) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported API_GetRegisteredPluginPresent()", CLR_RED);
        return YYTK_FAIL;
    };
    // Http API
    if (PmSetExported(pAttr, "API_HttpGetRequest", HttpRequests::API_HttpGetRequest) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported API_HttpGetRequest()", CLR_RED);
        return YYTK_FAIL;
    };

#pragma endregion

    Misc::Print("Exported functions correctly. Mods can load these now.", CLR_GOLD);

    // set version to modded
    double gv = static_cast<double>(Binds::CallBuiltinA("variable_global_get", {"game_version"}));
    Binds::CallBuiltinA("variable_global_set", { "game_version",std::format("{} modded", gv)});

    // update check
    Versioning::TriggerUpdateCheck("https://api.github.com/repos/sam-k0/LoopHeroCallbackCore/releases/latest", gPluginVersion, Versioning::UpdateCheckHttpCallback);


    Misc::Print("Creating button");
    gModsButtonRef = static_cast<double>(Binds::CallBuiltinA("instance_create_depth", {531., 330., -10010., (double)LHObjectEnum::o_menu_button}));
    Binds::CallBuiltinA("variable_instance_set", { gModsButtonRef, "click_event", -1. }); // delete original callback
    Binds::CallBuiltinA("variable_instance_set", { gModsButtonRef, "text", "Modding Hub" });
    Binds::CallBuiltinA("variable_instance_set", { gModsButtonRef, "fa_ltext", "Modding Hub" });
    
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
        if (GetAsyncKeyState(VK_F12) & 1)
        {   
            if(!gModuleManagerReady)continue;

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
        CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeyControls, NULL, 0, NULL));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

