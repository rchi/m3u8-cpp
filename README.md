# m3u8-cpp

This is a pure header files library used to parse M3U8.
Spec depends on [Apple's HTTP Live Streaming protocol](http://tools.ietf.org/html/draft-pantos-http-live-streaming).

I convert the [node-m3u8](https://github.com/tedconf/node-m3u8) from `javascript`
into `c++` by using `Github Copilot`.

All the test-cases passed but I am still not sure if everything works as expected.

## Build

```bash
sudo apt install nlohmann-json3-dev libcurl4-openssl-dev
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug && make -j
```

## Run

```bash
./m3u8-test
./m3u8-example ../test/fixtures/variant.m3u8
./m3u8-example https://muiplayer.js.org/media/media.m3u8
```

## Code Style

Need `clang-format-12` at least.

```bash
find . -name "*.h" | xargs clang-format -i
find . -name "*.cpp" | xargs clang-format -i
```
