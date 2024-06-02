#include <signal.h>

#include <condition_variable>
#include <thread>

#include "m3u8/UrlParser.h"

static std::atomic<bool> signal_received(false);

static void signalHandler(int signum) { signal_received.store(true); }

int main(int argc, char *argv[])
{
    signal(SIGINT, signalHandler);

    if (!strncmp(argv[1], "http://", 7 || !strncmp(argv[1], "https://", 8))) {
        std::string base       = argv[1];
        std::size_t last_slash = base.find_last_of('/');
        if (last_slash != std::string::npos) {
            base = base.substr(0, last_slash);
        }

        std::condition_variable cv;
        std::mutex cv_m;

        std::shared_ptr<UrlParser> parser = std::make_shared<UrlParser>(
            argv[1],
            [&](M3UParser::CallbackType_t type, std::shared_ptr<void> arg) {
                if (type == M3UParser::ItemCallback) {
                    std::shared_ptr<Item> i
                        = std::static_pointer_cast<Item>(arg);
                    // std::cerr << i->toString() << std::endl;
                    std::cerr << base << "/" << i->get<std::string>("uri")
                              << std::endl;
                }
                else if (type == M3UParser::M3u8Callback) {
                    std::cerr << "EOS" << std::endl;
                    std::shared_ptr<M3u8> m3u
                        = std::static_pointer_cast<M3u8>(arg);
                    // std::cerr << m3u->toString() << std::endl;
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
    }

    return 0;
}
