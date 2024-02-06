#pragma once
#include <fstream>
#include <iterator>
#include <map>
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