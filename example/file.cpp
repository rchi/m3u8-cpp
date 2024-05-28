#include "m3u8/FileParser.h"

int main(int argc, char *argv[])
{
    std::shared_ptr<FileParser> parser = std::make_shared<FileParser>();
    parser->parseFile(argv[1]);
    std::cerr << parser->m3u8()->toString() << std::endl;

    return 0;
}
