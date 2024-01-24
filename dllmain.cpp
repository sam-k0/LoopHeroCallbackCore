// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>

// YYTK is in this now
#include "MyHelper.h"

// Plugin functionality
#include <fstream>
#include <iterator>
#include <map>
#define _CRT_SECURE_NO_WARNINGS

std::map<std::string, YYTKPlugin*> gRegisteredPlugins;
bool gReady = false;

bool Ready() // Plugins call this to see if the core is present
{
    return gReady;
}

bool RegisterModule(std::string modName, YYTKPlugin* pluginHandle) // Plugins call this to register themselves to the core
{
    // Check if the module is already registered
    if (gRegisteredPlugins.find(modName) != gRegisteredPlugins.end())
    {
        Misc::Print("Module already registered", CLR_RED);
        return false;
    }

    // add to map
    gRegisteredPlugins.insert(std::pair<std::string, YYTKPlugin*>(modName, pluginHandle));
    Misc::Print("Registered callbacks for mod: " + modName, CLR_GREEN);
    return true;
}

bool UnregisterModule(std::string modName) // Plugins call this to say goodbye
{
    // Check if the module is registered at all
    if (gRegisteredPlugins.find(modName) == gRegisteredPlugins.end())
    {
        Misc::Print("Module not registered", CLR_RED);
        return false;
    }

    // remove from map
    gRegisteredPlugins.erase(modName);
    Misc::Print("Unregistered mod: " + modName, CLR_GREEN);
    return true;
}

void ShowWelcomeMessage()
{
    // Show a welcome message with multiple lines, but only as a single message
    Misc::Print("--------------------------\nCallback Core is loaded.\nPress F12 to list mods that registered callbacks.\n--------------------------", CLR_YELLOW);

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

    

    // Original event
    codeEvent->Call(selfInst, otherInst, codeObj, std::get<3>(codeEvent->Arguments()), std::get<4>(codeEvent->Arguments()));
   
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

    Misc::Print("Exporting function...", CLR_YELLOW);
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

    if (PmSetExported(pAttr, "CoreReady", Ready) != YYTK_OK)
    {
        Misc::Print("Failed to PmSetExported Ready()", CLR_RED);
        return YYTK_FAIL;
    };

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

    Misc::Print("Exported functions correctly", CLR_GREEN);
    return YYTK_OK; // Successful PluginEntry.
}

void PrintRegisteredMods()
{
// Show registered mods as a one-liner
    std::string mods = "[F12] Registered mods:\n";
    for (auto const& mod : gRegisteredPlugins)
    {
        mods += mod.first + "\n";
    }
    mods = mods.substr(0, mods.size() - 1);
    Misc::Print(mods, CLR_YELLOW);
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

