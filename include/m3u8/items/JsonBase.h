#pragma once
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class JsonBase : public json {
public:
    JsonBase(json props = json::object()) : json(json::object()) {}

    json get(std::string key) { return (*this)[toLowerCase(key)]; }

    void set(json* root, std::string key, json value)
    {
        // std::cout << "set(" << key << ", " << value << ")\n";
        if ((*root)[key].is_number()) {
            if (value.is_string()) {
                (*root)[key] = std::stoi(value.get<std::string>());
            }
            else {
                (*root)[key] = value;
            }
        }
        else if ((*root)[key].is_boolean()) {
            if (value.empty()) {
                (*root)[key] = true;
            }
            else if (value.is_string()) {
                std::string v = toLowerCase(value);
                if (v == "true" || v == "yes" || v == "") {
                    (*root)[key] = true;
                }
                else {
                    (*root)[key] = false;
                }
            }
            else {
                (*root)[key] = value;
            }
        }
        else if (key == "resolution" && value.is_string()) {
            std::string v         = value;
            std::string delimiter = "x";
            size_t pos            = 0;
            std::string token;
            int i = 0;
            while ((pos = v.find(delimiter)) != std::string::npos) {
                token = v.substr(0, pos);
                v.erase(0, pos + delimiter.length());
                (*root)[key][i] = std::stoi(token);
                i++;
            }
            (*root)[key][i] = std::stoi(v);
        }
        else {
            // std::cout << "set(" << key << ", " << value << ")\n";
            (*root)[key] = value;
        }
    }

    std::string toString(int i = 0) { return dump(i); }

protected:
    std::string toLowerCase(const std::string& str)
    {
        std::string lowerStr = str;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return lowerStr;
    }
};
