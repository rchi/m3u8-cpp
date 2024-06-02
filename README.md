# m3u8-cpp

This is a pure header files library used to parse M3U8.
Spec depends on [Apple's HTTP Live Streaming protocol](http://tools.ietf.org/html/draft-pantos-http-live-streaming).

I converted the [node-m3u8](https://github.com/tedconf/node-m3u8) from `javascript`
into `c++` by using `Github Copilot` then made some adjustments.

All the test-cases passed (yes, I deleted some of them)
but I am still not sure if everything works as expected.

TODO: Add `serialize` back for download function to save a local m3u8 file.
But I am not sure if `unserialize` is necessary or not.

## Build

```bash
sudo apt install nlohmann-json3-dev libcurl4-openssl-dev
mkdir build && cd build
cmake .. && make -j && make install
```

## Test

```bash
./test/m3u8-test
```

## Examples

Check local file.

```bash
./example/m3u8-file ../test/fixtures/variant.m3u8
```

Check online playlist.

```bash
./example/m3u8-url https://muiplayer.js.org/media/media.m3u8
```

Download m3u8 into one single file.

```bash
./example/m3u8-download https://muiplayer.js.org/media/media.m3u8 ~/Downloads/media.ts
```

Play m3u8 media.

```bash
./example/m3u8-player https://muiplayer.js.org/media/media.m3u8
```

## Code Style

Need `clang-format-12` at least.

```bash
find . -name "*.h" | xargs clang-format -i
find . -name "*.cpp" | xargs clang-format -i
```
