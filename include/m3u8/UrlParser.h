#pragma once

#include <curl/curl.h>

#include <m3u8/Parser.h>

class UrlParser : public M3UParser {
public:
    int parseURL(std::string uri)
    {
        CURL *curl;
        CURLcode res;
        std::string readBuffer;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();

        std::istringstream iss(readBuffer);
        std::string line;
        while (std::getline(iss, line)) {
            parse(line);
        }

        if (callback != NULL) {
            callback(M3u8Callback, this->m3u8());
        }
        return 0;
    }

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                                std::string *s)
    {
        size_t newLength = size * nmemb;
        size_t oldLength = s->size();
        try {
            s->resize(oldLength + newLength);
        } catch (std::bad_alloc &e) {
            return 0;
        }
        std::copy((char *)contents, (char *)contents + newLength,
                  s->begin() + oldLength);
        return size * nmemb;
    }
};
