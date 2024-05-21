#include "m3u8/FileParser.h"

int main(int argc, char *argv[])
{

    FileParser *parser = new FileParser;
    parser->parseFile(argv[1]);
    std::cerr << parser->m3u8()->toString() << std::endl;

    return 0;
}
