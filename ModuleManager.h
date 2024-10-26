#pragma once
#include <fstream>
#include <iterator>
#include <map>
#include "Filesystem.h"
#include "MyHelper.h"

std::map<std::string, YYTKPlugin*> gRegisteredPlugins;
bool gReady = false;
const std::string gModBlacklist = "DisabledMods.meta";
bool Ready() // Plugins call this to see if the core is present
{
    return gReady;
}

// Checks if the module is allowed or not
bool ModuleAllowed(std::string modName)
{  
    std::string blacklistedModsPath = Filesys::GetCurrentDir() + "\\" + gModBlacklist;
    if (Filesys::FileExists(blacklistedModsPath))
    {
        std::vector<std::string> blacklistedModsNames = Filesys::ReadFromFile(blacklistedModsPath);
        if (Misc::VectorContains(modName, &blacklistedModsNames)) // Blacklisted
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        Misc::Print("Creating Mods CFG", CLR_GOLD);
        std::ofstream file(blacklistedModsPath); // Create file without specifying content
        file.close();
        return true;
    }
    
}

bool RegisterModule(std::string modName, YYTKPlugin* pluginHandle) // Plugins call this to register themselves to the core
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
