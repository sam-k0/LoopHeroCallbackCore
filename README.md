# LoopHero CallbackCore

Module needed by my mods in order to function correctly.
Reason is that multiple mods cannot hook and then call the same game event, so this will handle it.

## API Functions

Plugins can import these functions via `PmGetExported` to interact with the CallbackCore system.

A simple example is here:

<details>
  <summary>Show simple code example</summary>
  
    ```cpp

    // First, define the function prototype for the API function you want to import
    using API_GetWindowHandleProto = HWND(*)();

    void* pFunc; // Get a raw pointer cause YYTK needs this...
    if (PmGetExported("API_GetWindowHandle", pFunc) == YYTK_OK)
    {
        API_GetWindowHandle = reinterpret_cast<API_GetWindowHandleProto>(pFunc); // reinterpret to the function proto-type
        Misc::Print("API_GetWindowHandle imported successfully", CLR_GREEN);
    }
    ```
</details>

To do this, just create a function prototype like this, the example is for `API_HttpGetRequest`:

<details>
  <summary>Show advanced code example</summary>
  
    ```cpp
     // The callback function for the HTTP event
    using HttpCallbackFn = void(*)(const char* responseString, int statusCode, int httpStatus);

    // This is the actual API function prototype
    using API_HttpGetRequestProto = void(*)(const char*, HttpCallbackFn, double);

    // Allocate a global function pointer variable to hold the imported function
    API_HttpGetRequestProto API_HttpGetRequest = nullptr;

    // define a callback function that matches the expected signature
    void MyHttpCallback(const char* responseString, int statusCode, int httpStatus)
    {
        // Print the response and status codes
        Misc::Print(std::format("HTTP Response: {}\nStatus Code: {}\nHTTP Status: {}", responseString, statusCode, httpStatus));
    }


    // Import the function using PmGetExported, usually in your InstallPatches function
    // You only want this to run once, so that's why it's a good place to do it.

    void* pFunc; // Get a raw pointer cause YYTK needs this...
    if (PmGetExported("API_HttpGetRequest", pFunc) == YYTK_OK)
    {
	    API_HttpGetRequest = reinterpret_cast<API_HttpGetRequestProto>(pFunc); // reinterpret to the function proto-type
        Misc::Print("API_HttpGetRequest imported successfully", CLR_GREEN);
    }

    // You can now call API_HttpGetRequest anywhere in your code!
    API_HttpGetRequest("https://jsonplaceholder.typicode.com/todos/1", MyHttpCallback,-1.);
    ```
</details>


### Code Event Patching (These are already imported if you are using the Template Mod)

#### `void API_InstallPrePatch(PrePostPatchCallback function)`
Registers a callback function to be executed **before** a game code event runs.

- **Parameters:**
  - `function`: A callback function of type `PrePostPatchCallback`
- **Return Value:** `void`
- **Behavior:** 
  - Multiple pre-patch callbacks can be registered by different plugins
  - Return `YYTK_DONTCALL` from your callback to prevent the original code from executing
  - Return `YYTK_OK` to allow the original code to run

#### `void API_InstallPostPatch(PrePostPatchCallback function)`
Registers a callback function to be executed **after** a game code event runs.

- **Parameters:**
  - `function`: A callback function of type `PrePostPatchCallback`
- **Return Value:** `void`
- **Behavior:**
  - Multiple post-patch callbacks can be registered by different plugins
  - Executed only if the original code was not cancelled by a pre-patch callback

### Window Management

#### `HWND API_GetWindowHandle()`
Retrieves the handle to the game's main window.

- **Return Value:** `HWND` - The game window handle
- **Usage:** Use this to interact with the game window directly (send messages, check focus, etc.)

### Plugin Enumeration

#### `int API_GetRegisteredPluginCount()`
Gets the total number of currently registered plugins.

- **Return Value:** `int` - The count of registered plugins
- **Usage:** Use this to determine the valid range for `API_GetRegisteredPluginName()`

#### `const char* API_GetRegisteredPluginName(int index)`
Retrieves the name of a registered plugin by its index.

- **Parameters:**
  - `index`: Zero-based index (0 to count-1)
- **Return Value:** `const char*` - Plugin name, or empty string if index is invalid
- **Example:** Loop from 0 to `API_GetRegisteredPluginCount() - 1` to enumerate all plugins

#### `bool API_GetRegisteredPluginPresent(const char* name)`
Checks if a plugin with the given name is currently registered.

- **Parameters:**
  - `name`: The name of the plugin to search for
- **Return Value:** `bool` - `true` if the plugin is registered, `false` otherwise
- **Usage:** Use this to check for the presence of a specific plugin before calling its exported functions

### HTTP Requests

#### `void API_HttpGetRequest(const char* url, HttpCallbackFn callbackFn, double headerMapId = BuildGitHubHeaders())`
Sends an HTTP GET request to the specified URL and invokes a callback when the request completes.

- **Parameters:**
  - `url`: The URL to send the GET request to
  - `callbackFn`: A callback function of type `HttpCallbackFn` to be invoked when the response is received
  - `headerMapId`: A GameMaker ds_map containing custom HTTP headers. Pass `-1` to use the default GitHub API headers. The function will handle destroying this map after the request is sent, so you do not need to manage its memory.
- **Return Value:** `void`
- **Callback Signature:** `void callback(const char* responseString, int statusCode, int httpStatus)`
  - `responseString`: A string containing the response, empty if not ready yet.
  - `statusCode`: Status code (0 for success, 1 for still loading, negative values for errors)
  - `httpStatus`: The HTTP status code (e.g., 200, 404, 500)
- **Behavior:**
  - Requests are asynchronous; your callback will be invoked when the response is received
  - If no custom headers are provided, the function uses default GitHub API headers