#pragma once
#include <fstream>
#include <iterator>
#include <map>
#include "Filesystem.h"
#include "MyPlugin.h"

static std::map<std::string, YYTKPlugin*> gRegisteredPlugins;
static bool gModuleManagerReady = false;

bool API_ModuleManagerReadyCheck() // Plugins call this to see if the core is present
{
    return gModuleManagerReady;
}

bool API_RegisterModule(std::string modName, YYTKPlugin* pluginHandle) // Plugins call this to register themselves to the core
{
    // Check if the module is already registered
    if (gRegisteredPlugins.find(modName) != gRegisteredPlugins.end())
    {
        PrintMessage(CLR_RED,"Mod %s already registered!", modName);
        return false;
    }
    // add to map
    gRegisteredPlugins.insert(std::pair<std::string, YYTKPlugin*>(modName, pluginHandle));

	PrintMessage(CLR_GOLD,"Registered mod: %s", modName);
    return true;
    
}

bool API_UnregisterModule(std::string modName) // Plugins call this to say goodbye
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

int API_GetRegisteredPluginCount()
{
    return gRegisteredPlugins.size();
}

const char* API_GetRegisteredPluginName(int index)
{
    if (index < 0 || index >= gRegisteredPlugins.size())
    {
        return "";
	}

	return gRegisteredPlugins.at(std::next(gRegisteredPlugins.begin(), index)->first)->PluginEntry ? std::next(gRegisteredPlugins.begin(), index)->first.c_str() : "";
}

bool API_GetRegisteredPluginPresent(const char* name)
{
	// search gRegisteredPlugins for name
    for (auto const& mod : gRegisteredPlugins)
    {
        if (mod.first == name)
        {
            return true;
        }
    }
	return false;
}