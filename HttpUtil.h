#pragma once
#include "MyPlugin.h"
#include <map>
#include <string>
#include <algorithm>
#include <regex>
#include <format>
#include "YYRValueParse.h"

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


namespace Versioning
{
    // parses version tags in format of "v1.2.3" or "version 1.2.3" to an integer for easy comparison, returns -1 if invalid
    int ParseVersionTag(const std::string& tag)
    {
        std::regex versionRegex(R"((\d+)\.(\d+)\.(\d+))");
        std::smatch match;
        if (std::regex_search(tag, match, versionRegex) && match.size() == 4)
        {
            int major = std::stoi(match[1]);
            int minor = std::stoi(match[2]);
            int patch = std::stoi(match[3]);
            return major * 10000 + minor * 100 + patch;
        }
        return -1; // Invalid version tag
    }

    void TriggerUpdateCheck(const char* url, int currentVersionNumber, HttpRequests::HttpCallbackFn callbackFn)
    {
        HttpRequests::API_HttpGetRequest(url, callbackFn, -1.);
	}

    void ShowGenericNotice()
    {   
        double updateinfo = static_cast<double>(Binds::CallBuiltinA("instance_create_depth", { 270.0,480.0 / 2, 0.0, (double)LHObjectEnum::o_menu_message }));
        Binds::CallBuiltinA("variable_instance_set", { updateinfo, "text_message", std::format("Mod framework successfully initialized!").c_str() });
        Binds::CallBuiltinA("variable_instance_set", { updateinfo, "text", "Okay" });
    }

    void UpdateCheckHttpCallback(const char* responseString, int statusCode, int httpStatus)
    {
        bool checkFailed = false;
        // check response codes
        if (statusCode == 1)// still loading
        {
            return ShowGenericNotice();
        }
        if (statusCode < 0)// error, maybe offline?
        {
            Misc::Print(std::format("Failed to check for updates. StatusCode: {}, HttpStatus: {}", statusCode, httpStatus));
            return ShowGenericNotice();
        }
        // statuscode is 0 now
		if (httpStatus != 200)
        {
            Misc::Print(std::format("Failed to check for updates: Http Error: {}", httpStatus));
            return ShowGenericNotice();
        }
        // All errors are catched now

         
        // load as dsmap
		YYRValue jsonMap = Binds::CallBuiltinA("json_decode", { responseString });
        // get top level tag_name key
		YYRValue tagName = Binds::CallBuiltinA("ds_map_find_value", { jsonMap, "tag_name" });
		std::string tagNameStr = YYRValueParse::DCS(tagName);
        Binds::CallBuiltinA("ds_map_destroy", { jsonMap });
		
        // parse it and compare to current version
		int remoteVersion = ParseVersionTag(tagNameStr);
        if (remoteVersion == -1)
        {
            Misc::Print(std::format("Failed to parse version tag from response: {}", tagNameStr));
            return ShowGenericNotice();
		}
        
		if (remoteVersion > gPluginVersion)
        {
            Misc::Print(std::format("A new version of the plugin is available! Latest version: {}. Please check the github releases page to update!", tagNameStr));
            double updateinfo = static_cast<double>(Binds::CallBuiltinA("instance_create_depth", { 270.0,480.0 / 2, 0.0, (double)LHObjectEnum::o_menu_message }));
            Binds::CallBuiltinA("variable_instance_set", { updateinfo, "text_message", std::format("Mod framework successfully initialized!\n\n\nUpdate notice:\n\nA new version of Callback Core is available.\nVersion {} can be downloaded from the GitHub releases.", tagNameStr).c_str()});
            Binds::CallBuiltinA("variable_instance_set", { updateinfo, "text", "Okay" });
        }
        else
        {
            Misc::Print(std::format("You are using the latest version of the plugin! {} superseeds {}", gPluginVersion, remoteVersion));
            return ShowGenericNotice();
        }
		
    }
}
