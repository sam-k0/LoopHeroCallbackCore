#pragma once
#include "MyPlugin.h"
#include <map>
#include <string>
#include <algorithm>

#define HTTP_EVENT_ID "gml_Object_oCloudSaves_Other_62"
namespace HttpRequests
{
	using HttpCallbackFn = void(*)(const char* responseString, int statusCode, int httpStatus); // responseMap(ds_map), statusCode(0 for success, 1 for still loading, <0 for error), httpStatus (HTTP status code)
	static std::map<int, HttpCallbackFn> gPluginHttpEventIds; // EventId, CallbackFn

	// Checks if a http event is custom, if yes, removes it from the list and returns true, otherwise returns false
    bool IsCustomHttpEvent(int eventId)
    {
		return gPluginHttpEventIds.contains(eventId);
	}

    void HandleHttpEvent(YYRValue asyncLoadMap)
    {        
        // Get the HttpEventID, HttpStatusCode
		YYRValue eventId = Binds::CallBuiltinA("ds_map_find_value", { asyncLoadMap, "id" });
		YYRValue gmStatus = Binds::CallBuiltinA("ds_map_find_value", { asyncLoadMap, "status" });
		YYRValue httpStatus = Binds::CallBuiltinA("ds_map_find_value", { asyncLoadMap, "http_status" });
        
        if ((double)gmStatus == 0.0) // Done downloading
        {
            YYRValue resultKeyExists = Binds::CallBuiltinA("ds_map_exists", { asyncLoadMap, "result" });
            if ((double)resultKeyExists == 1.0)
            {
                YYRValue responseString = Binds::CallBuiltinA("ds_map_find_value", { asyncLoadMap, "result" });
                // Call the callback function with the result map and status code 0 for success
                gPluginHttpEventIds[(int)eventId](std::string(static_cast<const char*>(responseString)).c_str(), (double)gmStatus, (double)httpStatus);
            }
            else
            {
                // Call the callback function with an empty map and status code 0 for success (no result)
                gPluginHttpEventIds[(int)eventId]("", (double)gmStatus, (int)httpStatus);
            }
			// Clean up the event id from the map
			gPluginHttpEventIds.erase((int)eventId);
        } 
		else // Error or still loading
        {
			gPluginHttpEventIds[(int)eventId]("", (double)gmStatus, (int)httpStatus);
        }
    }

    // Required headers for github api
    YYRValue BuildGitHubHeaders()
    {
        YYRValue headers = Binds::CallBuiltinA("ds_map_create", {});

        Binds::CallBuiltinA("ds_map_add", { headers, "User-Agent",
            "Mozilla/5.0 (X11; Linux x86_64; rv:150.0) Gecko/20100101 Firefox/150.0"
            });

        Binds::CallBuiltinA("ds_map_add", { headers, "Accept",
            "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"
            });

        Binds::CallBuiltinA("ds_map_add", { headers, "Accept-Encoding",
            "gzip, deflate, br, zstd"
            });

        Binds::CallBuiltinA("ds_map_add", { headers, "Accept-Language",
            "en-US,en;q=0.9"
            });

        Binds::CallBuiltinA("ds_map_add", { headers, "Connection",
            "keep-alive"
            });

        Binds::CallBuiltinA("ds_map_add", { headers, "Host",
            "api.github.com"
            });

        Binds::CallBuiltinA("ds_map_add", { headers, "Priority",
            "u=0, i"
            });

        Binds::CallBuiltinA("ds_map_add", { headers, "Sec-Fetch-Dest",
            "document"
            });

        Binds::CallBuiltinA("ds_map_add", { headers, "Sec-Fetch-Mode",
            "navigate"
            });

        Binds::CallBuiltinA("ds_map_add", { headers, "Sec-Fetch-Site",
            "none"
            });

        Binds::CallBuiltinA("ds_map_add", { headers, "Sec-Fetch-User",
            "?1"
            });

        Binds::CallBuiltinA("ds_map_add", { headers, "Sec-GPC",
            "1"
            });

        Binds::CallBuiltinA("ds_map_add", { headers, "Upgrade-Insecure-Requests",
            "1"
            });

        return headers;
    }


    void API_HttpGetRequest(const char* url, HttpCallbackFn callbackFn, double headerMapId)
    {
        if (headerMapId == -1.0)
        {
            headerMapId = BuildGitHubHeaders();
        }
        YYRValue eventId = Binds::CallBuiltinA("http_get", {url, "GET", (double)headerMapId, "" });
        Binds::CallBuiltinA("ds_map_destroy", { headerMapId });
		gPluginHttpEventIds.insert({ (int)eventId, callbackFn });
    }
}