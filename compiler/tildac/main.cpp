#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include <tildac/types.hpp>
#include <tildac/parser.hpp>

int main(int argc, char** argv) {
    for(tildac::i64 i = 1; i < argc; ++i) {
        tildac::parse_file(argv[i]);
    }
    return 0;
}
