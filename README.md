# LoopHero CallbackCore

Module needed by my mods in order to function correctly.
Reason is that multiple mods cannot hook and then call the same game event, so this will handle it.



Exposes various functions for other plugins to call, import them via PmGetExported:



* void API\_InstallPostPatch(PrePostPatchCallback function)
* void API\_InstallPrePatch(PrePostPatchCallback function)
* HWND API\_GetWindowHandle()
* int API\_GetRegisteredPluginCount()
* const char\* API\_GetRegisteredPluginName(int index)

