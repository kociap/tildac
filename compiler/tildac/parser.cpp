#include <tildac/parser.hpp>

#include <tildac/utility.hpp>
#include <tildac/types.hpp>
#include <tildac/ast.hpp>

#include <vector>
#include <iostream>
#include <string>
#include <fstream>

namespace tildac {
    enum class Token_Category {
        eof,
        keyword,
        separator,
        operator_,
        identifier,
        // literal,
    };

    enum class Keyword {
        kw_fn,
        kw_if,
        kw_else,
        kw_switch,
        kw_case,
        kw_for,
        kw_while,
        kw_do,
        kw_return,
        kw_break,
        kw_continue,
        kw_void,
        kw_bool,
        kw_c8,
        kw_c16,
        kw_c32,
        kw_i8,
        kw_u8,
        kw_i16,
        kw_u16,
        kw_i32,
        kw_u32,
        kw_i64,
        kw_u64,
        kw_f32,
        kw_f64,
        kw_mut,
    };

    enum class Separator {
        brace_open,
        brace_close,
        bracket_open,
        bracket_close,
        paren_open,
        paren_close,
        angle_open,
        angle_close,
        drill,
        semicolon,
        colon,
        scope_resolution,
        comma,
        question,
        dot,
    };

    // TODO: Add compound operators (+=, -=, etc.) to the try_match function.
    // TODO: call operator, array access operator, elvis operator
    enum class Operator {
        plus,
        minus,
        multiply,
        divide,
        modulo,
        logic_and,
        bit_and,
        logic_or,
        bit_or,
        bit_xor,
        logic_negation,
        bit_negation,
        bit_lshift,
        bit_rshift,
        equal,
        not_equal,
        less,
        greater,
        less_equal,
        greater_equal,
        assign,
        address,
        dereference,
        drill,
        increment,
        decrement,
        compound_plus,
        compound_minus,
        compound_multiply,
        compound_divice,
        compound_modulo,
        compound_bit_and,
        compound_bit_or,
        compound_bit_xor,
        compound_bit_lshift,
        compound_bit_rshift,
    };

    enum class Literal {
        bool_literal,
        integer_literal,
        float_literal,
        string_literal,
    };

    union Token_Type {
        Separator separator;
        Keyword keyword;
        Literal literal;
        Operator operator_;
    };

    static std::string_view stringify_token_category(Token_Category t) {
        switch(t) {
            case Token_Category::keyword: return "keyword";
            case Token_Category::separator: return "separator";
            case Token_Category::operator_: return "operator";
            case Token_Category::identifier: return "identifier";
            // case Token_Category::literal: return "literal";
        }
    }

    struct Token {
        Token_Category category;
        Token_Type type;
        // TODO: string interning.
        std::string name;
    };

    static bool is_whitespace(char32 c) {
        return (c >= 0 & c <= 32) | (c == 127);
    }

    static bool is_digit(char32 c) {
        return c >= 48 && c < 58;
    }

    static bool is_alpha(char32 c) {
        return (c >= 97 && c < 123) || (c >= 65 && c < 91);
    }

    static bool is_first_identifier_character(char32 c) {
        return c == '_' || is_alpha(c);
    }

    static bool is_identifier_character(char32 c) {
        return c == '_' || is_digit(c) || is_alpha(c);
    }

    class Lexer_State {
    public:
        i64 stream_offset;
        i64 line;
        i64 column;
    };

    class Lexer {
    public:
        Lexer(std::istream& file): _stream(file) {}

        bool try_match_separator(Separator const separator) {
            static constexpr std::string_view separators[] = {
                "{", "}", "[", "]", "(", ")", "<", ">", "->", ";", ":", "::", ",",
                "?", "."
            };

            ignore_whitespace_and_comments();

            Lexer_State const state_backup = get_current_state();
            std::string_view const str = separators[static_cast<i64>(separator)];
            for(auto i = str.begin(), end = str.end(); i != end; ++i) {
                if(get_next() != *i) {
                    restore_state(state_backup);
                    return false;
                }
            }
            return true;
        }

        bool try_match_operator(Operator const operator_) {
            static constexpr std::string_view operators[] = {
                "+", "-", "*", "/", "%", "&&", "&", "||", "|", "^", "!", "~", "<<", ">>",
                "==", "!=", "<", ">", "<=", ">=", "=", "@", "*", "->", "++", "--",
                "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="
            };

            ignore_whitespace_and_comments();

            Lexer_State const state_backup = get_current_state();
            std::string_view const str = operators[static_cast<i64>(operator_)];
            for(auto i = str.begin(), end = str.end(); i != end; ++i) {
                if(get_next() != *i) {
                    restore_state(state_backup);
                    return false;
                }
            }
            return true;
        }

        bool try_match_keyword(Keyword const keyword) {
            static constexpr std::string_view keywords[] = {
                "fn", "if", "else", "switch", "case" "for", "while", "do", "return", "break", "continue",
                "void", "bool", "c8", "c16", "c32", "i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "f32", "f64",
                "mut"
            };

            ignore_whitespace_and_comments();

            Lexer_State const state_backup = get_current_state();
            std::string_view const str = keywords[static_cast<i64>(keyword)];
            for(auto i = str.begin(), end = str.end(); i != end; ++i) {
                if(get_next() != *i) {
                    restore_state(state_backup);
                    return false;
                }
            }
            return true;
        }

        bool try_match_identifier(std::string& out) {
            ignore_whitespace_and_comments();

            // No need to backup the lexer state since we can predict whether the next
            // sequence of characters is an identifier using only the first character.
            char32 const next_char = peek_next();
            if(!is_first_identifier_character(next_char)) {
                return false;
            }

            get_next();
            out += next_char;
            for(char32 peek = peek_next(); is_identifier_character(peek); peek = peek_next()) {
                out += peek;
                get_next();
            }
            return true;
        }

        bool try_match_eof() {
            ignore_whitespace_and_comments();
            char32 const next_char = peek_next();
            return next_char == std::char_traits<char>::eof();
        }

        void ignore_whitespace_and_comments() {
            while(true) {
                char32 const next_char = peek_next();
                if(is_whitespace(next_char)) {
                    get_next();
                    continue;
                }

                if(next_char == U'/') {
                    get_next();
                    char32 const next_next_char = peek_next();
                    if(next_next_char == U'/') {
                        get_next();
                        while(get_next() != U'\n') {}
                    } else if(next_next_char == U'*') {
                        get_next();
                        for(char32 c1 = get_next(), c2 = peek_next(); c1 != U'*' || c2 != U'/'; c1 = get_next(), c2 = _stream.peek()) {}
                        get_next();
                    } else {
                        unget();
                    }
                    continue;
                }

                break;
            }
        }

        Lexer_State get_current_state() const {
            return {_stream.tellg(), _line, _column};
        }

        void restore_state(Lexer_State const state) {
            _stream.seekg(state.stream_offset, std::ios_base::beg);
            _line = state.line;
            _column = state.column;
        }

    private:
        std::istream& _stream;
        i64 _line = 0;
        i64 _column = 0;

        char32 get_next() {
            char32 const c = _stream.get();
            if(c == '\n') {
                _line += 1;
                _column = 0;
            } else {
                _column += 1;
            }
            return c;
        }

        char32 peek_next() {
            return _stream.peek();
        }

        void unget() {
            _stream.unget();
        }
    };

    class Parser {
    public:
        Parser(std::istream& stream): _lexer(stream) {}

        void build_ast() {
            while(true) {
                if(Declaration_Sequence* decl_seq = try_declaration_sequence(); decl_seq) {
                    decl_seq->print(std::cout, Indent{0});
                    delete decl_seq;
                }

                if(_lexer.try_match_eof()) {
                    break;
                }
            }   
        }

    private:
        Lexer _lexer;
        
        Declaration_Sequence* try_declaration_sequence() {
            Declaration_Sequence* const declaration_sequence = new Declaration_Sequence;
            while(true) {
                Declaration* declaration = try_declaration();
                if(declaration) {
                    declaration_sequence->append_declaration(declaration);
                } else {
                    break;
                }
            }

            i64 const declaration_count = declaration_sequence->get_declarations_count();
            if(declaration_count > 0) {
                return declaration_sequence;
            } else {
                delete declaration_sequence;
                return nullptr;
            }
        }

        Declaration* try_declaration() {
            if(Function_Declaration* function_declaration = try_function_declaration(); function_declaration) {
                return function_declaration;
            }

            return nullptr;
        }

        Function_Declaration* try_function_declaration() {
            Lexer_State const state_backup = _lexer.get_current_state();
            if(!_lexer.try_match_keyword(Keyword::kw_fn)) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Identifier* name = nullptr;
            if(std::string fn_name; _lexer.try_match_identifier(fn_name)) {
                name = new Identifier(std::move(fn_name));
            } else {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(!_lexer.try_match_separator(Separator::paren_open)) {
                _lexer.restore_state(state_backup);
                delete name;
                return nullptr;
            }

            // TODO: Parameter list.

            if(!_lexer.try_match_separator(Separator::paren_close)) {
                _lexer.restore_state(state_backup);
                delete name;
                return nullptr;
            }

            if(!_lexer.try_match_separator(Separator::drill)) {
                _lexer.restore_state(state_backup);
                delete name;
                return nullptr;
            }

            // TODO: Implement proper types.
            Type_Name* return_type = nullptr;
            if(std::string return_name; _lexer.try_match_identifier(return_name)) {
                return_type = new Type_Name();
            } else {
                _lexer.restore_state(state_backup);
                delete name;
                return nullptr;
            }

            Function_Body* function_body = try_function_body();
            if(!function_body) {
                _lexer.restore_state(state_backup);
                delete name;
                delete return_type;
                return nullptr;
            }

            return new Function_Declaration(name, return_type, function_body);
        }

        Function_Body* try_function_body() {
            if(_lexer.try_match_separator(Separator::brace_open) && _lexer.try_match_separator(Separator::brace_close)) {
                return new Function_Body;
            } else {
                return nullptr;
            }
        }
    };


    void parse_file(std::string_view const path) {
        std::cout << "Opening " << path << " for reading" << std::endl;
        std::string const path_str(path);
        std::ifstream file(path_str);
        if(!file) {
            std::cout << "Could not open " << path << "\n";
            return;
        }
        
        Parser parser(file);
        parser.build_ast();
    }
}
