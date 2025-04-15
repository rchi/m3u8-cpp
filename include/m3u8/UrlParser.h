#pragma once

#include <curl/curl.h>
#include <m3u8/Parser.h>

#include <atomic>
#include <thread>

class UrlParser : public M3UParser {
public:
    UrlParser() : run(false) {}

    UrlParser(std::string uri, CallbackFunc callback) : UrlParser()
    {
        setCallback(callback);
        parseURL(uri);
    }

    int parseURL(std::string uri, int max = 0)
    {
        run = true;
        t   = std::thread([this, uri, max]() {
            pthread_setname_np(/*pthread_self(),*/ "UrlParser");

            CURL *curl;
            std::string readBuffer;

            curl_global_init(CURL_GLOBAL_DEFAULT);
            curl = curl_easy_init();
            if (curl) {
                curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            }

            while (run) {
                if (curl) {
                    CURLcode res = curl_easy_perform(curl);
                    if (res != CURLE_OK) {
                        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                                curl_easy_strerror(res));
                    }

                    readNext(readBuffer, max);

                    int interval = 1;
                    try {
                        json tgt = m3u8()->get("targetDuration");
                        interval = (int)tgt;

                    } catch (...) {
                        std::cerr << "OnError: " << m3u8()->toString()
                                  << std::endl;
                    }

                    std::this_thread::sleep_for(std::chrono::seconds(interval));
                }
            }

            if (curl) {
                curl_easy_reset(curl);
                curl_easy_cleanup(curl);
            }
            curl_global_cleanup();
        });
        t.detach();
        return 0;
    }

    void stop()
    {
        run = false;
        if (t.joinable()) {
            t.join();
        }
    }

    void readNext(std::string readBuffer, int max = 0)
    {
        M3UParser parser;
        std::istringstream iss(readBuffer);
        std::string line;
        while (std::getline(iss, line)) {
            parser.parse(line);
        }
        merge(parser.m3u8(), max);

        // std::cout << "readNext: " << readBuffer << "\n"
        //           << m3u8()->toString() << "\n";
        if (callback != NULL) {
            callback(M3u8Callback, m3u8());
        }
    }

private:
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

private:
    std::thread t;
    std::atomic<bool> run;
};
