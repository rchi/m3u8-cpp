#include <gtest/gtest.h>

std::string path = "../test/fixtures/";

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::FLAGS_gtest_break_on_failure = false;

    if (argc > 1) {
        path = argv[1];
        std::cerr << "Load test file from " << path << "\n";
    }

    return RUN_ALL_TESTS();
}
