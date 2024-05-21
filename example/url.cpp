#include <condition_variable>
#include <thread>

#include "m3u8/UrlParser.h"

int main(int argc, char *argv[])
{
    if (!strncmp(argv[1], "http://", 7 || !strncmp(argv[1], "https://", 8))) {
        std::condition_variable cv;
        std::mutex cv_m;
        bool done = false;

        UrlParser *parser = new UrlParser;
        parser->setCallback([&](M3UParser::CallbackType_t type, void *) {
            if (type == M3UParser::ItemCallback) {
            }
            else if (type == M3UParser::M3u8Callback) {
                if (parser->m3u8()->get("playlistType") == "VOD") {
                    done = true;
                    cv.notify_one();
                }
            }
        });
        while (!done) {
            parser->parseURL(argv[1]);
            std::cerr << parser->m3u8()->toString() << std::endl;
            if (!done) {
                /** In fact, this interval is wrong.
                 *  We should do more math-work to get a better interval at the
                 *  end of the playlist.
                 *  But now, it's only for retry.
                 */
                int interval = (int)parser->getCurrentItem()->get("duration");
                if (interval > 1) interval /= 2;
                std::this_thread::sleep_for(std::chrono::seconds(interval));
            }
        }

        std::unique_lock<std::mutex> lk(cv_m);
        cv.wait(lk, [&] { return done; });

        delete parser;
    }

    return 0;
}