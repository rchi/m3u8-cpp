#include <signal.h>

#include <chrono>
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

extern "C" {
#include <SDL2/SDL.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include "m3u8/UrlParser.h"

class ElapsedTimer {
public:
    ElapsedTimer(std::string f)
        : func(f), start_time_(std::chrono::high_resolution_clock::now())
    {
    }

    ~ElapsedTimer()
    {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time_;
        auto now        = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);

        std::tm *now_tm = std::localtime(&now_time_t);
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%H:%M:%S", now_tm);

        std::cout << "[" << buffer << "]";
        std::cout << "[" << std::hex << std::this_thread::get_id() << "]";
        std::cout << func << "() Elapsed time: " << std::fixed
                  << std::setprecision(9) << elapsed.count() << " seconds.\n";
    }

private:
    std::string func;
    std::chrono::high_resolution_clock::time_point start_time_;
};
#define DBG_ElapsedTimer(f) ElapsedTimer timer(f);

static std::atomic<bool> run;
static void signalHandler(int signum) { run.store(false); }

int main(int argc, char *argv[])
{
    int i;
    bool firstFrame = true;

    int videoStream                 = -1;
    AVFormatContext *pFormatCtx     = NULL;
    AVFormatContext *pNextFormatCtx = NULL;
    AVCodecContext *pVideoCodecCtx  = NULL;
    AVCodec *pVideoCodec            = NULL;
    AVFrame *pFrame                 = NULL;
    AVFrame *pScaledFrame           = NULL;
    AVPacket packet;
    std::string nextMedia = "";

    SDL_Window *screen        = NULL;
    SDL_Renderer *sdlRenderer = NULL;
    SDL_Texture *sdlTexture   = NULL;
    SDL_DisplayMode dm;

    int targetWidth;
    int targetHeight;

    std::shared_ptr<UrlParser> parser;
    std::string base = argv[1];
    std::vector<std::string> playlist;
    static std::recursive_mutex playlist_mutex;

    if (argc < 2) {
        printf("Please provide a movie file\n");
        return -1;
    }

    signal(SIGINT, signalHandler);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER
                    | SDL_INIT_EVENTS)) {
        SDL_Log("Could not initialize SDL - %s", SDL_GetError());
        return -1;
    }

    auto destroySurface = [&]() {
        DBG_ElapsedTimer("destroySurface");
        if (sdlTexture != NULL) {
            SDL_DestroyTexture(sdlTexture);
        }
        if (sdlRenderer != NULL) {
            SDL_DestroyRenderer(sdlRenderer);
        }
        if (screen != NULL) {
            SDL_DestroyWindow(screen);
        }
    };

    auto createSurface = [&](int w, int h) {
        targetWidth  = w;
        targetHeight = h;
        int ret      = 0;
        if (screen == NULL) {
            DBG_ElapsedTimer("createSurface");
            do {
                ret = -1;

                if (SDL_GetCurrentDisplayMode(0, &dm) != 0) {
                    SDL_Log("SDL_GetCurrentDisplayMode failed: %s",
                            SDL_GetError());
                    break;
                }
                if (w > dm.w) {
                    float scale  = (float)dm.w / w;
                    targetWidth  = dm.w;
                    targetHeight = static_cast<int>(h * scale);
                }

                screen = SDL_CreateWindow(
                    "My Video Window", SDL_WINDOWPOS_UNDEFINED,
                    SDL_WINDOWPOS_UNDEFINED, targetWidth, targetHeight,
                    SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
                        | SDL_WINDOW_RESIZABLE);
                if (!screen) {
                    SDL_Log("SDL: could not set media mode - exiting\n");
                    break;
                }
                SDL_MaximizeWindow(screen);

                sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
                if (sdlRenderer == NULL) {
                    SDL_Log("SDL: could not create renderer - exiting\n");
                    break;
                }
                sdlTexture = SDL_CreateTexture(
                    sdlRenderer, SDL_PIXELFORMAT_IYUV,
                    SDL_TEXTUREACCESS_STREAMING, targetWidth, targetHeight);
                if (sdlTexture == NULL) {
                    SDL_Log("SDL: could not create texture - exiting\n");
                    break;
                }
                ret = 0;
            } while (0);

            if (ret == -1) {
                destroySurface();
            }
        }
        return ret;
    };

    auto waitUserAction = [&]() {
        DBG_ElapsedTimer("waitUserAction");
        // Main loop
        SDL_Event event;
        while (run) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    run = false;
                    break;
                }
            }

            if (!run) break;
            SDL_Delay(100);
        }
    };

    auto createPlaylist = [&]() {
        if (!strncmp(argv[1], "http://",
                     7 || !strncmp(argv[1], "https://", 8))) {
            static std::atomic<bool> signal_received(false);
            std::condition_variable cv;
            std::mutex cv_m;

            std::size_t last_slash = base.find_last_of('/');
            if (last_slash != std::string::npos) {
                base = base.substr(0, last_slash);
            }
            parser = std::make_shared<UrlParser>(
                argv[1],
                [&](M3UParser::CallbackType_t type, std::shared_ptr<void> arg) {
                    if (type == M3UParser::ItemCallback) {
                        std::lock_guard<std::recursive_mutex> lock(
                            playlist_mutex);
                        std::shared_ptr<Item> i
                            = std::static_pointer_cast<Item>(arg);
                        // std::cerr << i->toString() << std::endl;
                        std::string uri
                            = base + "/" + i->get<std::string>("uri");
                        playlist.push_back(uri);
                        std::cout << uri << std::endl;
                    }
                    else if (type == M3UParser::M3u8Callback) {
                        signal_received = true;
                        cv.notify_one();
                    }
                });
            while (!signal_received.load()) {
                std::unique_lock<std::mutex> lk(cv_m);
                if (cv.wait_for(lk, std::chrono::seconds(1),
                                [&] { return signal_received.load(); })) {
                    break;
                }
            }
        }
        else {
            playlist.push_back(base);
        }
    };

    auto findCodecCtx = [&](AVMediaType type, int *index) {
        DBG_ElapsedTimer("findCodecCtx");
        AVCodecContext *pCodecCtx = NULL;
        do {
            // Find the first media stream
            if (((*index)
                 = av_find_best_stream(pFormatCtx, type, -1, -1, NULL, 0))
                == -1) {
                fprintf(stderr, "Didn't find a media stream\n");
                break;  // Didn't find a media stream
            }

            // Get a pointer to the codec parameters for the media stream
            AVCodecParameters *pCodecPar
                = pFormatCtx->streams[(*index)]->codecpar;

            // Allocate a codec context for the decoder
            pCodecCtx = avcodec_alloc_context3(NULL);
            if (!pCodecCtx) {
                fprintf(stderr, "Could not allocate media codec context\n");
                break;
            }

            // Copy codec parameters from input stream to output codec context
            if (avcodec_parameters_to_context(pCodecCtx, pCodecPar) < 0) {
                fprintf(stderr, "Could not copy codec parameters\n");
                pCodecCtx = NULL;
                break;
            }
        } while (0);
        return pCodecCtx;
    };

    auto openDecoder = [&](AVCodecContext *pCodecCtx) {
        DBG_ElapsedTimer("openDecoder");
        AVCodec *pCodec = NULL;
        do {
            // Find the decoder for the media stream
            pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
            if (pCodec == NULL) {
                fprintf(stderr, "Unsupported codec!\n");
                break;  // Codec not found
            }
            // Open codec
            if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
                fprintf(stderr, "Failed to open codec!\n");
                pCodec = NULL;
                break;  // Could not open codec
            }
        } while (0);
        return pCodec;
    };

    auto scaleVideo = [&]() {
        if (pVideoCodecCtx->width > targetWidth
            || pVideoCodecCtx->height > targetHeight) {
            SwsContext *pSwsCtx = sws_getContext(
                pVideoCodecCtx->width, pVideoCodecCtx->height,
                pVideoCodecCtx->pix_fmt, targetWidth, targetHeight,
                AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
            if (pSwsCtx) {
                av_image_alloc(pScaledFrame->data, pScaledFrame->linesize,
                               targetWidth, targetHeight, AV_PIX_FMT_YUV420P,
                               1);

                sws_scale(pSwsCtx, (uint8_t const *const *)pFrame->data,
                          pFrame->linesize, 0, pVideoCodecCtx->height,
                          pScaledFrame->data, pScaledFrame->linesize);
                sws_freeContext(pSwsCtx);
            }
            return pScaledFrame;
        }
        else {
            return pFrame;
        }
    };

    auto drawFrame = [&]() {
        AVFrame *frame = scaleVideo();
        SDL_UpdateYUVTexture(sdlTexture, NULL, frame->data[0],
                             frame->linesize[0], frame->data[1],
                             frame->linesize[1], frame->data[2],
                             frame->linesize[2]);

        SDL_Rect srcRect;
        srcRect.x = 0;
        srcRect.y = 0;
        srcRect.w = targetWidth;
        srcRect.h = targetHeight;

        SDL_Rect dstRect;
        dstRect.x = (dm.w > targetWidth) ? ((dm.w - targetWidth) / 2) : 0;
        dstRect.y = (dm.h > targetHeight) ? ((dm.h - targetHeight) / 2) : 0;
        dstRect.w = targetWidth;
        dstRect.h = targetHeight;

        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, sdlTexture, &srcRect, &dstRect);
        SDL_RenderPresent(sdlRenderer);

        av_freep(&pScaledFrame->data);
    };

    auto delay = [&]() {
        int video_fps = pFormatCtx->streams[videoStream]->avg_frame_rate.num
                        / pFormatCtx->streams[videoStream]->avg_frame_rate.den;
        SDL_Delay(1000 / video_fps);
    };

    auto decodeAndDraw = [&]() {
        if (firstFrame) {
            std::cerr << "decodeAndDraw() firstFrame\n";
            firstFrame = false;
        }
        // Send packet to decoder
        if (avcodec_send_packet(pVideoCodecCtx, &packet) < 0) {
            fprintf(stderr, "Error sending packet for decoding\n");
            return -1;
        }

        while (true) {
            int ret;
            // Receive frame from decoder
            if ((ret = avcodec_receive_frame(pVideoCodecCtx, pFrame))
                == AVERROR(EAGAIN)) {
                // Need more input to produce a frame
                break;
            }
            else if (ret < 0) {
                fprintf(stderr, "Error during decoding\n");
                return -1;
            }

            // Frame is successfully decoded
            drawFrame();
            delay();
        }
        return 0;
    };

    auto openMedia = [&]() {
        AVFormatContext *pFmtCtx = NULL;
        std::lock_guard<std::recursive_mutex> lock(playlist_mutex);

        do {
            if (playlist.empty()) {
                break;
            }

            std::string media = "";
            media             = playlist.front();
            DBG_ElapsedTimer("openMedia(" + media + ")");

            if (avformat_open_input(&pFmtCtx, media.c_str(), NULL, NULL) != 0) {
                break;  // Couldn't open file
            }
            if (avformat_find_stream_info(pFmtCtx, NULL) < 0) {
                break;  // Couldn't find stream information
            }

            if (media != "") {
                playlist.erase(playlist.begin());
            }
            nextMedia = media;
        } while (0);

        return pFmtCtx;
    };

    auto closeMedia = [&]() {
        DBG_ElapsedTimer("closeMedia");
        avcodec_close(pVideoCodecCtx);
        avcodec_free_context(&pVideoCodecCtx);
        if (pFormatCtx != NULL) {
            avformat_close_input(&pFormatCtx);
            pFormatCtx = NULL;
        }
    };

    auto swapNextMedia = [&]() {
        DBG_ElapsedTimer("swapNextMedia");
        if (pNextFormatCtx == NULL && playlist.empty()) {
            std::cerr << "no more.\n";
            return -1;
        }
        else {
            std::cerr << "Swap to " << nextMedia << std::endl;
            pFormatCtx     = pNextFormatCtx;
            pNextFormatCtx = NULL;
            nextMedia      = "";
        }
        firstFrame = true;
        return 0;
    };

    createPlaylist();

    run = true;
    std::thread preload;
    std::thread t([&]() {
        pNextFormatCtx = openMedia();
        swapNextMedia();

        while (run) {
            if (!playlist.empty() || pFormatCtx != NULL) {
                if (pFormatCtx != NULL) {
                    preload = std::thread([&]() {
                        std::cerr << "preload started.\n";
                        std::lock_guard<std::recursive_mutex> lock(
                            playlist_mutex);
                        if (!playlist.empty()) {
                            if ((pNextFormatCtx = openMedia()) == NULL) {
                                std::cerr << "Failed to preload media.\n";
                            }
                        }
                        else {
                            pNextFormatCtx = NULL;
                            std::cerr << "No media left.\n";
                        }
                        DBG_ElapsedTimer("preload");
                    });

                    if ((pVideoCodecCtx
                         = findCodecCtx(AVMEDIA_TYPE_VIDEO, &videoStream))
                        == NULL) {
                        break;
                    }
                    if (createSurface(pVideoCodecCtx->width,
                                      pVideoCodecCtx->height)
                        < 0) {
                        break;
                    }
                    if ((pVideoCodec = openDecoder(pVideoCodecCtx)) == NULL) {
                        break;
                    }

                    // Allocate media frame
                    pFrame       = av_frame_alloc();
                    pScaledFrame = av_frame_alloc();

                    // Main loop
                    while (run && av_read_frame(pFormatCtx, &packet) >= 0) {
                        // Is this a packet from the media stream?
                        if (packet.stream_index == videoStream) {
                            if (decodeAndDraw() < 0) {
                                break;
                            }
                        }
                        // Free the packet that was allocated by av_read_frame
                        av_packet_unref(&packet);
                    }

                    av_frame_free(&pFrame);
                    av_frame_free(&pScaledFrame);

                    closeMedia();

                    if (preload.joinable()) {
                        preload.join();
                    }
                    if (swapNextMedia() < 0) {
                        break;
                    }
                }
                else {
                    SDL_Delay(100);
                    swapNextMedia();
                }
            }
        }
        destroySurface();
        std::cerr << "Finished.\n";
        run = false;
        SDL_Quit(); // call in each thread
    });

    waitUserAction();

    if (t.joinable()) {
        t.join();
    }
    SDL_Quit(); // call in each thread

    return 0;
}
