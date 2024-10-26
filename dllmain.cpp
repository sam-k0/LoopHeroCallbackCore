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
#include <map>
#include <chrono>
#define _CRT_SECURE_NO_WARNINGS

YYRValue modsbutton = -4.0;
YYRValue modsInfo = -4.;
bool buttonHovered = false;
int lastSurf = -1;
bool active = false;
std::tuple<double, double, double, double> materialHoverBounds = std::make_tuple(9999, 9999, 0, 0);
// Clock
auto lastTime = std::chrono::high_resolution_clock::now();

std::vector<std::string> blacklistPluginNames;

void ShowWelcomeMessage()
{
    // Show a welcome message with multiple lines, but only as a single message
    Misc::Print("Press F12 to list registered mods.", CLR_GOLD);

}
// Unload
YYTKStatus PluginUnload()
{
    PmRemoveCallback(callbackAttr);

    return YYTK_OK;
}


void findMaterialBorders()
{
    // iterate over all instances of o_camp_resource_main
    YYRValue instNum = Binds::CallBuiltinA("instance_number", { double(LHObjectEnum::o_camp_resource_main) });
    for (int i = 0; i < (int)instNum; ++i)
    {
        YYRValue currentInstance = Binds::CallBuiltinA("instance_find", { double(LHObjectEnum::o_camp_resource_main), double(i) });
        // get x and y
        double x = Binds::CallBuiltin("variable_instance_get", nullptr, nullptr, { currentInstance, "x" });
        double y = Binds::CallBuiltin("variable_instance_get", nullptr, nullptr, { currentInstance, "y" });
        //PrintMessage(CLR_RED, "Bounds: %f %f , %f", x, y, currentInstance);

        YYRValue spr = Binds::CallBuiltinA("variable_instance_get", { currentInstance, "sprite_index" });
        double w, h;
        Assets::GetSpriteDimensions(spr, w, h);

        // check if bounds are uninitialized
        {
            // in tuple, first two are top left corner, second two are bottom right corner
            // Check if the current instance's x and y are smaller than the top left corner or bigger than the bottom right corner
            if (x < std::get<0>(materialHoverBounds) || y < std::get<1>(materialHoverBounds))
            {
                // replace top left corner
                materialHoverBounds = std::make_tuple(x, y, std::get<2>(materialHoverBounds), std::get<3>(materialHoverBounds));
            }
            // check if the current instance's x and y are bigger than the bottom right corner
            if (x + w / 2 > std::get<2>(materialHoverBounds) || y + h / 2 > std::get<3>(materialHoverBounds))
            {
                // replace bottom right corner
                materialHoverBounds = std::make_tuple(std::get<0>(materialHoverBounds), std::get<1>(materialHoverBounds), x + w / 2, y + h / 2);
            }
        }
    }
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


    

#pragma region ModManager
    // Mouse events
    
    if (selfInst->i_spriteindex == (int)modsbutton)
    {
        if (Misc::StringHasSubstr(codeObj->i_pName, "o_base_button_Mouse_10")) // Enter
        {
            buttonHovered = true;
        }
        if (Misc::StringHasSubstr(codeObj->i_pName, "o_base_button_Mouse_11")) //Leave
        {
            buttonHovered = false;
        }
    }
   
    // Event for entering the camp 
    if (Misc::StringHasSubstr(codeObj->i_pName, "gml_Room_rm_camp_Create"))
    {
        // create button
        modsbutton = Binds::CallBuiltinA("instance_create_depth", { -999.,170.,-200.,(double)LHObjectEnum::o_base_button });
        Binds::CallBuiltinA("variable_instance_set", { modsbutton, "sprite_index", (double)LHSpriteEnum::s_camp_buildingb });

        PrintMessage(CLR_RED, "Created mod management button with oID: %d", (int)modsbutton);
        // load blacklist
        blacklistPluginNames = Filesys::ReadFromFile(Filesys::GetCurrentDir() + "\\" + gModBlacklist);
    }


    // draw event?
    if (Misc::StringHasSubstr(codeObj->i_pName, "o_menu_button_Draw_0"))//
    {
        if ((bool)Binds::CallBuiltinA("instance_exists", { modsbutton }))
        {
            double first = Binds::CallBuiltinA("instance_find", { (double)LHObjectEnum::o_menu_button, 0. });
            if (double(selfInst->i_spriteindex) == first)
            {
                // draw self (button)
                double x = Binds::CallBuiltinA("variable_instance_get", { modsbutton, "x" });
                double y = Binds::CallBuiltinA("variable_instance_get", { modsbutton, "y" });
                YYRValue spr = Binds::CallBuiltinA("variable_instance_get", { modsbutton , "sprite_index" });
                YYRValue subimg = Binds::CallBuiltinA("variable_instance_get", { modsbutton, "image_index" });

                Binds::CallBuiltinA("draw_sprite", {spr, subimg, x, y});

                // Draw text to button
                YYRValue halign = Binds::CallBuiltinA("draw_get_halign", {  });
                YYRValue valign = Binds::CallBuiltinA("draw_get_valign", {  });
                YYRValue font = Binds::CallBuiltinA("draw_get_font", {});
                YYRValue depth = Binds::CallBuiltinA("variable_instance_get", { double(selfInst->i_spriteindex), "depth" });

                Binds::CallBuiltinA("draw_set_color", { 16777215.0 });
                Binds::CallBuiltinA("draw_set_halign", { 1.0 });
                Binds::CallBuiltinA("draw_set_valign", { 1.0 });

                double w, h;
                Assets::GetSpriteDimensions(spr, w, h);
                Binds::CallBuiltinA("draw_text_transformed", { x + w / 2,y + h / 2.3,"Mods",.5,.5,0.0 });
                Binds::CallBuiltinA("draw_set_halign", { halign });
                Binds::CallBuiltinA("draw_set_valign", { valign });
                Binds::CallBuiltinA("draw_set_font", { font });
            }
        }
    }

    if (Misc::StringHasSubstr(codeObj->i_pName, "o_base_button_Mouse_56"))
    {
        if (buttonHovered)
        {
            if ((bool)Binds::CallBuiltinA("instance_exists", { modsInfo }) == false && !(bool)Binds::CallBuiltinA("instance_exists", { (double)LHObjectEnum::o_camp_statistik }))
            {
                //Misc::Print("created");
                modsInfo = Binds::CallBuiltinA("instance_create_depth", { 32.,32.,-999.,double(LHObjectEnum::o_camp_statistik) });
                YYRValue scrollerStruct = Binds::CallBuiltinA("variable_instance_get", { modsInfo, "scroller" });
                Binds::CallBuiltinA("variable_struct_set", { scrollerStruct, "active", 0.0 }); // Dont allow scrolling
                
                // Create list of buttons with blacklisted / not blacklisted content
            }
        }
    }

   

    if (Misc::StringHasSubstr(codeObj->i_pName, "o_menu_Step_0"))
    {
        if ((bool)Binds::CallBuiltinA("instance_exists", { modsbutton }))
        {
            // Find a ui button
            YYRValue anchorButton = Binds::CallBuiltinA("instance_find", { (double)LHObjectEnum::o_menu_TW, 0.0});
            if ((double)anchorButton != INSTANCE_NOONE)
            {
                YYRValue tbx = Binds::CallBuiltinA("variable_instance_get", { anchorButton , "x"});
                YYRValue tby = Binds::CallBuiltinA("variable_instance_get", { anchorButton , "y" });

                // Set modsbutton pos to that + offs
                Binds::CallBuiltinA("variable_instance_set", { modsbutton, "x", 480. + 60 });
                Binds::CallBuiltinA("variable_instance_set", { modsbutton, "y", (double)tby + 16 });
            }
        }
    }


    if (Misc::StringHasSubstr(codeObj->i_pName, "statistik_Draw_0"))
    {
        //PrintMessage(CLR_DEFAULT, "%s SpriteID: %d", codeObj->i_pName, selfInst->i_spriteindex);
        if (selfInst->i_spriteindex == (int)modsInfo)
        {
            // Check if surf exists, free it, create new, assign
            YYRValue statsPanel = modsInfo;//Binds::CallBuiltinA("instance_nearest", { 32.0,32.0,double(LHObjectEnum::o_camp_statistik) });
            YYRValue origsurf = Binds::CallBuiltinA("variable_instance_get", { statsPanel, "statsurf" });
            Binds::CallBuiltinA("variable_instance_set", { statsPanel, "text","Manage Mods" });
            if ((int)origsurf != lastSurf)
            {
                YYRValue exists = Binds::CallBuiltinA("surface_exists", { origsurf });
                if ((bool)exists == true) //Draw
                {
                    //Misc::Print("surf exists");
                    if ((bool)Binds::CallBuiltinA("surface_set_target", { origsurf }) == true)
                    {
                        // save orig draw vars
                        YYRValue halign = Binds::CallBuiltinA("draw_get_halign", {  });
                        YYRValue font = Binds::CallBuiltinA("draw_get_font", {});
                        // get surf vars
                        YYRValue surfw = Binds::CallBuiltinA("surface_get_width", { origsurf });
                        YYRValue surfh = Binds::CallBuiltinA("surface_get_height", { origsurf });

                        std::string text = "Activate or deactivate mods here!\nToggle mods by pressing [F11]\nand input the mod runtime ID\n\nRegistered Mods:\n";
                        int counter = 0;
                        for (auto const& mod : gRegisteredPlugins)
                        {
                            text += mod.first + " : " + std::to_string(counter) + "\n";
                            counter++;
                        }
                        if (gRegisteredPlugins.size() == 0)
                        {
                            text += "*No mods active\n";
                        }
                        counter = 0;
                        text += "\nDeactivated Mods:\n";
                        for (auto const& mod : blacklistPluginNames)
                        {
                            text += mod + " : " + std::to_string(counter)+"\n";
                            counter++;
                        }
                        if (blacklistPluginNames.size() == 0)
                        {
                            text += "*No mods deactivated\n";
                        }
                        text = text.substr(0, text.size() - 1);

                        // draw
                        Binds::CallBuiltinA("draw_set_font", { 1.0 });

                        YYRValue col = Binds::CallBuiltinA("make_color_rgb", { 96.,34.,23. });
                        Binds::CallBuiltinA("draw_set_color", { col });
                        Binds::CallBuiltinA("draw_rectangle", { 0.0,0.0,surfw, surfh, 0.0 });

                        col = Binds::CallBuiltinA("make_color_rgb", { 255.,255.,255. });
                        Binds::CallBuiltinA("draw_set_color", { 16777215.0 });
                        Binds::CallBuiltinA("draw_set_halign", { 0.0 });
                        Binds::CallBuiltinA("draw_text_transformed", { 32.,32.,text,1.0,1.0,0.0 });
                        // reset
                        Binds::CallBuiltinA("surface_reset_target", { });
                        Binds::CallBuiltinA("draw_set_halign", { halign });
                        Binds::CallBuiltinA("draw_set_font", { font });
                    }
                }
            }
            lastSurf = (int)origsurf;
        }
    }

#pragma endregion

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

void ShowModToggleDialog()
{
    int idx = 0;
    std::vector<std::string> keys;
    int res = 0;
    // Show a dialog asking if you want to activate or deactivate a mod
    HWND hwnd = NULL;
    LPCWSTR lpText = L"Press [Yes] if you want to activate a mod, [No] if you want to deactivate a mod, [Cancel] to do nothing.";
    LPCWSTR lpCaption = L"Manage mods";
    UINT uType = MB_YESNOCANCEL | MB_ICONINFORMATION;
    int nResult = MessageBox(hwnd, lpText, lpCaption, uType);

    switch (nResult) {
    case IDYES:
        res = 1;
        break;
    case IDNO:
        res = 2;
        break;
    case IDCANCEL:
        // cancel
        break;
    default:
        break;
    }

    if (res == 1)
    {
        idx = (double)Binds::CallBuiltinA("get_integer", { "Index of mod to activate", -1.0 });
        if (idx < 0 || idx >= blacklistPluginNames.size())
        {
            Misc::Print("Invalid index", CLR_RED);
            return;
        }
        // remove from blacklist
        blacklistPluginNames.erase(blacklistPluginNames.begin() + idx);
        // write new blacklist to file
        Filesys::WriteToFile(Filesys::GetCurrentDir() + "\\" + gModBlacklist, blacklistPluginNames);
    }
    else if(res == 2)
    {
        idx = (double)Binds::CallBuiltinA("get_integer", { "Index of mod to disable", -1.0 });
        if (idx < 0 || idx >= gRegisteredPlugins.size())
        {
            std::string errtext = "Invalid index: " + std::to_string(idx) + " Max index: " + std::to_string(gRegisteredPlugins.size());
            Misc::Print(errtext, CLR_RED);

            return;
        }
        // add to blacklist
        // first, iterate through map and add to the vector of strings

        for (auto const& mod : gRegisteredPlugins)
        {
            keys.push_back(mod.first);
        }
        blacklistPluginNames.push_back(keys[idx]);
        Filesys::WriteToFile(Filesys::GetCurrentDir() + "\\" + gModBlacklist, blacklistPluginNames);
    }


    // Refresh lists
    blacklistPluginNames = Filesys::ReadFromFile(Filesys::GetCurrentDir() + "\\" + gModBlacklist);
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
        if (GetAsyncKeyState(VK_F11))
        {
            if (!gReady)continue;

            // List all registered mods
            // check if stats window exists
            if ((bool)Binds::CallBuiltinA("instance_exists", { modsInfo }) == true)
            {
                ShowModToggleDialog();
            }
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

