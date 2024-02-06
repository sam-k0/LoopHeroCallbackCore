// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>

// YYTK is in this now
#include "MyHelper.h"
#include "ModuleManager.h"
#include "LHObjects.h"
#include "LHSprites.h"
#include "Assets.h"
// Plugin functionality
#include <fstream>
#include <iterator>
#include <map>
#define _CRT_SECURE_NO_WARNINGS

YYRValue modsbutton = -4.0;
YYRValue modsInfo = -4.;
bool buttonHovered = false;
int lastSurf = -1;
bool active = false;


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

#pragma region ModManager
    // Mouse events
    
    if (Misc::StringHasSubstr(codeObj->i_pName, "o_base_button_Mouse_10")) // Enter
    {
        if (selfInst->i_spriteindex == (int)modsbutton)
        {
            buttonHovered = true;
        }
    }

    if (Misc::StringHasSubstr(codeObj->i_pName, "o_base_button_Mouse_11")) //Leave
    {
        if (selfInst->i_spriteindex == (int)modsbutton)
        {
            buttonHovered = false;
        }
    }
    
    // Event for entering the camp
    if (Misc::StringHasSubstr(codeObj->i_pName, "gml_Room_rm_camp_Create"))
    {
        // create button
        modsbutton = Binds::CallBuiltinA("instance_create_depth", { 531.,170.,-10009.,(double)LHObjectEnum::o_base_button });
        Binds::CallBuiltinA("variable_instance_set", { modsbutton, "sprite_index", (double)LHSpriteEnum::s_camp_buildingb });

        PrintMessage(CLR_RED, "Created btn %d", (int)modsbutton);
    }

    if (Misc::StringHasSubstr(codeObj->i_pName, "o_camp_game_button_Draw"))
    {

        // Draw text to button
        YYRValue halign = Binds::CallBuiltinA("draw_get_halign", {  });
        YYRValue font = Binds::CallBuiltinA("draw_get_font", {});
        YYRValue depth = Binds::CallBuiltinA("variable_instance_get", { double(selfInst->i_spriteindex), "depth" });
        //Misc::Print((int)depth);
        Binds::CallBuiltinA("draw_set_color", { 16777215.0 });
        Binds::CallBuiltinA("draw_set_halign", { 1.0 });

        //Binds::CallBuiltinA("draw_set_font", { 1.0 });

        double x = Binds::CallBuiltinA("variable_instance_get", { modsbutton, "x" });
        double y = Binds::CallBuiltinA("variable_instance_get", { modsbutton, "y" });
        YYRValue spr = Binds::CallBuiltinA("variable_instance_get", { modsbutton , "sprite_index" });
        double w, h;
        Assets::GetSpriteDimensions(spr, w, h);


        Binds::CallBuiltinA("draw_text_transformed", { x + w / 2,y + h / 2.3,"Mods",.5,.5,0.0 });

        Binds::CallBuiltinA("draw_set_halign", { halign });
        Binds::CallBuiltinA("draw_set_font", { font });
    }

    if (Misc::StringHasSubstr(codeObj->i_pName, "o_base_button_Mouse_56"))
    {
        if (buttonHovered)
        {
            if ((bool)Binds::CallBuiltinA("instance_exists", { modsInfo }) == false && !(bool)Binds::CallBuiltinA("instance_exists", { (double)LHObjectEnum::o_camp_statistik }))
            {
                Misc::Print("created");
                modsInfo = Binds::CallBuiltinA("instance_create_depth", { 32.,32.,-999.,double(LHObjectEnum::o_camp_statistik) });
                YYRValue scrollerStruct = Binds::CallBuiltinA("variable_instance_get", { modsInfo, "scroller" });
                Binds::CallBuiltinA("variable_struct_set", { scrollerStruct, "active", 0.0 }); // Dont allow scrolling
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
            Binds::CallBuiltinA("variable_instance_set", { statsPanel, "text","Playing modded" });
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

                        std::string text = "Registered Mods:\n";
                        
                        for (auto const& mod : gRegisteredPlugins)
                        {
                            text += mod.first + "\n";
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

