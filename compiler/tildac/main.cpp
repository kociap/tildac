#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <tildac/ast.hpp>
#include <tildac/codegen.hpp>
#include <tildac/parser.hpp>
#include <tildac/types.hpp>

int main(int argc, char** argv) {
    for(tildac::i64 i = 1; i < argc; ++i) {
        anton::Expected<Owning_Ptr<Declaration_Sequence>, Parse_Error> res = tildac::parse_file(argv[i]);
        if(!res) {
            Parse_Error const& error = res.error();
            std::cout << argv[i] << ":" << error.line << ":" << error.column << ": error:" << error.message << '\n';
            return -1;
        }

        generate(res.value()->decls, true);
    }
    return 0;
}
