#include <curl/curl.h>
#include <signal.h>

#include <condition_variable>
#include <filesystem> // need c++17
#include <fstream>
#include <mutex>
#include <thread>

#include "m3u8/UrlParser.h"

static std::atomic<bool> signal_received(false);

static void signalHandler(int signum) { signal_received.store(true); }

static size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    return fwrite(ptr, size, nmemb, stream);
}

static bool download_file(const std::string& url, const std::string& filepath)
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    FILE* fp = fopen(filepath.c_str(), "ab");
    if (!fp) {
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    CURLcode res = curl_easy_perform(curl);

    fclose(fp);
    curl_easy_cleanup(curl);

    return res == CURLE_OK;
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf("%s <url> <downloads>\n", argv[0]);
        return -1;
    }

    signal(SIGINT, signalHandler);

    if (!strncmp(argv[1], "http://", 7 || !strncmp(argv[1], "https://", 8))) {
        std::string base       = argv[1];
        std::size_t last_slash = base.find_last_of('/');
        if (last_slash != std::string::npos) {
            base = base.substr(0, last_slash);
        }
        std::string download = argv[2];

        std::condition_variable cv;
        std::mutex cv_m;
        std::vector<std::string> playlist;
        static std::recursive_mutex playlist_mutex;

        std::thread t([&]() {
            std::string uri = "";
            while (!signal_received || !playlist.empty()) {
                if (!playlist.empty()) {
                    std::lock_guard<std::recursive_mutex> lock(playlist_mutex);
                    uri = playlist.front();
                    playlist.erase(playlist.begin());
                }
                if (!uri.empty()) {
                    std::string filepath;
                    if (std::filesystem::is_directory(download)) {
                        std::string filename
                            = uri.substr(uri.find_last_of("/") + 1);
                        filepath = download + "/" + filename;
                    }
                    else {
                        filepath = download;
                    }
                    download_file(uri, filepath);
                }
                else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        });

        std::shared_ptr<UrlParser> parser = std::make_shared<UrlParser>(
            argv[1],
            [&](M3UParser::CallbackType_t type, std::shared_ptr<void> arg) {
                if (type == M3UParser::ItemCallback) {
                    std::lock_guard<std::recursive_mutex> lock(playlist_mutex);
                    std::shared_ptr<Item> i
                        = std::static_pointer_cast<Item>(arg);
                    std::string uri = base + "/" + i->get<std::string>("uri");
                    std::cout << uri << std::endl;
                    playlist.push_back(uri);
                }
                else if (type == M3UParser::M3u8Callback) {
                    std::shared_ptr<M3u8> m3u
                        = std::static_pointer_cast<M3u8>(arg);
                    if (m3u->get("playlistType") == "VOD") {
                        signal_received = true;
                        cv.notify_one();
                    }
                }
            });

        while (!signal_received.load()) {
            std::unique_lock<std::mutex> lk(cv_m);
            if (cv.wait_for(lk, std::chrono::seconds(1),
                            [&] { return signal_received.load(); })) {
                break;
            }
        }

        parser->stop();
        if (t.joinable()) {
            t.join();
        }
    }

    return 0;
}
