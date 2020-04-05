#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <cstdint>
#include <vector>

using u8 = unsigned char;
using i64 = std::int64_t;

enum class Token_Type {
    keyword,
    separator,
    oper,
    identifier,
    bool_literal,
    integer_literal,
    float_literal,
    string_literal,
};

static std::string_view stringify_token_type(Token_Type t) {
    switch(t) {
        case Token_Type::keyword: return "keyword";
        case Token_Type::separator: return "separator";
        case Token_Type::oper: return "oper";
        case Token_Type::identifier: return "identifier";
        case Token_Type::bool_literal: return "bool_literal";
        case Token_Type::integer_literal: return "integer_literal";
        case Token_Type::float_literal: return "float_literal";
        case Token_Type::string_literal: return "string_literal";
    }
}

struct Token {
    Token_Type type;
    std::string str;
};

static bool is_whitespace(char c) {
    return (c >= 0 & c <= 32) | (c == 127);
}

static bool is_valid_first_identifier(char c) {
    return (c >= 'A' & c <= 'Z') ||
           (c >= 'a' & c <= 'z') ||
           c == '_';
}

static bool is_valid_identifier(char c) {
    return (c >= 'A' & c <= 'Z') ||
           (c >= 'a' & c <= 'z') ||
           (c >= '0' & c <= '9') ||
           c == '_';
}

static bool is_operator(char c) {
    return c == '-' || c == '+' || c == '/' || c == '*' || c == '%' ||
           c == '>' || c == '<' || c == '=' || c == '|' || c == '&' || c == '^' || c == '!' ||
           c == '@';
}

static bool is_separator(char c) {
    return c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == ';' || c == ',';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_bool_literal(std::string_view identifier) {
    return identifier == "true" || identifier == "false";
}

static std::string_view const keywords[] = { "fn", "if", "else", "switch", "case" "for", "while", "do", "return", "break", "continue",
                                             "bool", "c8", "c16", "c32", "i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "f32", "f64",
                                             "mut" };

static void compile(std::string_view path) {
    std::cout << "Opening " << path << " for reading" << std::endl;
    std::string const path_str(path);
    std::ifstream file(path_str);
    if(!file) {
        std::cout << "Could not open " << path << "\n";
        return;
    }
    file.seekg(0, std::ios::end);
    i64 const file_size = file.tellg();;
    file.seekg(0, std::ios::beg);
    std::vector<char> file_contents(file_size + 1, 0);
    file.read(file_contents.data(), file_size);
    file.close();
    file_contents[file_contents.size() - 1] = -1;

    std::cout << "File read\nLexing " << path << std::endl;

    std::vector<Token> tokens;

    for(i64 index = 0; true;) {
        while(is_whitespace(file_contents[index])) {
            index += 1;
        }

        // integer literal and float literal of the form 
        if(is_digit(file_contents[index]) || file_contents[index] == '.' && is_digit(file_contents[index + 1])) {
            bool has_dot = file_contents[index] == '.';
            Token token;
            token.type = has_dot ? Token_Type::float_literal : Token_Type::integer_literal;
            token.str.push_back(file_contents[index]);
            index += 1;
            while(is_digit(file_contents[index]) || file_contents[index] == '.') {
                if(file_contents[index] == '.') {
                    if(has_dot) {
                        std::cout << "Invalid float literal: multiple decimal separators." << std::endl;
                        return;
                    }

                    has_dot = true;
                    token.type = Token_Type::float_literal;
                }
                
                token.str.push_back(file_contents[index]);
                index += 1;
            }

            tokens.push_back(std::move(token));
            continue;
        }

        // string literal
        if(file_contents[index] == '"') {
            Token token;
            token.type = Token_Type::string_literal;
            do {
                token.str.push_back(file_contents[index]);
                ++index;
            } while(file_contents[index] != '"' || file_contents[index - 1] == '\\');
            token.str.push_back('"');
            ++index;
            tokens.push_back(std::move(token));
            continue;
        }

        if(is_valid_first_identifier(file_contents[index])) {
            Token token;
            i64 index_backup = index;
            token.type = Token_Type::identifier;
            token.str.push_back(file_contents[index]);
            index += 1;
            while(is_valid_identifier(file_contents[index])) {
                token.str.push_back(file_contents[index]);
                index += 1;
            }

            if(is_bool_literal(token.str)) {
                token.type = Token_Type::bool_literal;
            } else {
                for(i64 i = 0; i < (sizeof(keywords) / sizeof(keywords[0])); ++i) {
                    if(token.str == keywords[i]) {
                        token.type = Token_Type::keyword;
                        break;
                    }
                }
            }

            tokens.push_back(std::move(token));
            continue;
        }

        if(is_operator(file_contents[index])) {
            Token token;
            token.type = Token_Type::oper;
            token.str.push_back(file_contents[index]);
            index += 1;
            while(is_operator(file_contents[index])) {
                token.str.push_back(file_contents[index]);
                index += 1;
            }
            tokens.push_back(std::move(token));
            continue;
        }

        if(is_separator(file_contents[index])) {
            Token token;
            token.type = Token_Type::separator;
            token.str.push_back(file_contents[index]);
            tokens.push_back(std::move(token));
            index += 1;
            continue;
        }

        if(file_contents[index] == -1) {
            break;
        }

        std::cout << "Unknown symbol " << file_contents[index] << std::endl;
        return;
    }

    std::cout << "Lexed file " << path << ". Tokens: \n";

    for(Token& token: tokens) {
        std::cout << stringify_token_type(token.type) << ": " << token.str << '\n';
    }
}

int main(int argc, char** argv) {
    for(i64 i = 1; i < argc; ++i) {
        compile(argv[i]);
    }
    return 0;
}
