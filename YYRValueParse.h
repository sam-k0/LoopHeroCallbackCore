#pragma once
#include "SDK/SDK.hpp"


namespace YYRValueParse
{
    std::string DCS(YYRValue val)
    {
        // Try to get C string from YYRValue
        const char* strPtr = static_cast<const char*>(val);

        if (strPtr)
        {
            // std::string constructor makes a deep copy
            return std::string(strPtr);
        }
        else
        {
            return std::string();
        }
    }

    std::string YYRValueToString(const YYRValue& val)
    {
        YYRValue type;
        CallBuiltin(type, "typeof", nullptr, nullptr, { val });
        std::string typeStr = DCS(type);
        if (typeStr == "number")
            return std::to_string(double(val));
        else if (typeStr == "bool")
            return bool(val) ? "true" : "false";
        else if (typeStr == "string")
            return DCS(val);
        else if (typeStr == "array")
            return "<array>";
        else
            return "<unknown>";
    }
}