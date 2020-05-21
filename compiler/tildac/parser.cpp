#include <tildac/parser.hpp>

#include <tildac/utility.hpp>
#include <tildac/types.hpp>
#include <tildac/ast.hpp>

#include <vector>
#include <iostream>
#include <string_view>
#include <string>
#include <fstream>

namespace tildac {
    // TODO: call operator, array access operator, elvis operator


    // keywords
    static constexpr std::string_view kw_fn = "fn";
    static constexpr std::string_view kw_if = "if";
    static constexpr std::string_view kw_else = "else";
    static constexpr std::string_view kw_switch = "switch";
    static constexpr std::string_view kw_case = "case";
    static constexpr std::string_view kw_for = "for";
    static constexpr std::string_view kw_while = "while";
    static constexpr std::string_view kw_do = "do";
    static constexpr std::string_view kw_return = "return";
    static constexpr std::string_view kw_break = "break";
    static constexpr std::string_view kw_continue = "continue";
    static constexpr std::string_view kw_mut = "mut";
    static constexpr std::string_view kw_var = "var";
    static constexpr std::string_view kw_true = "true";
    static constexpr std::string_view kw_false = "false";
    // builtin types
    static constexpr std::string_view token_void = "void";
    static constexpr std::string_view token_bool = "bool";
    static constexpr std::string_view token_c8 = "c8";
    static constexpr std::string_view token_c16 = "c16";
    static constexpr std::string_view token_c32 = "c32";
    static constexpr std::string_view token_i8 = "i8";
    static constexpr std::string_view token_u8 = "u8";
    static constexpr std::string_view token_i16 = "i16";
    static constexpr std::string_view token_u16 = "u16";
    static constexpr std::string_view token_i32 = "i32";
    static constexpr std::string_view token_u32 = "u32";
    static constexpr std::string_view token_i64 = "i64";
    static constexpr std::string_view token_u64 = "u64";
    static constexpr std::string_view token_f32 = "f32";
    static constexpr std::string_view token_f64 = "f64";
    // separators and operators
    static constexpr std::string_view token_brace_open = "{";
    static constexpr std::string_view token_brace_close = "}";
    static constexpr std::string_view token_bracket_open = "[";
    static constexpr std::string_view token_bracket_close = "]";
    static constexpr std::string_view token_paren_open = "(";
    static constexpr std::string_view token_paren_close = ")";
    static constexpr std::string_view token_angle_open = "<";
    static constexpr std::string_view token_angle_close = ">";
    static constexpr std::string_view token_semicolon = ";";
    static constexpr std::string_view token_colon = ":";
    static constexpr std::string_view token_scope_resolution = "::";
    static constexpr std::string_view token_comma = ",";
    static constexpr std::string_view token_question = "?";
    static constexpr std::string_view token_dot = ".";
    static constexpr std::string_view token_plus = "+";
    static constexpr std::string_view token_minus = "-";
    static constexpr std::string_view token_multiply = "*";
    static constexpr std::string_view token_divide = "/";
    static constexpr std::string_view token_modulo = "%";
    static constexpr std::string_view token_logic_and = "&&";
    static constexpr std::string_view token_bit_and = "&";
    static constexpr std::string_view token_logic_or = "||";
    static constexpr std::string_view token_bit_or = "|";
    static constexpr std::string_view token_bit_xor = "^";
    static constexpr std::string_view token_logic_negation = "!";
    static constexpr std::string_view token_bit_negation = "~";
    static constexpr std::string_view token_bit_lshift = "<<";
    static constexpr std::string_view token_bit_rshift = ">>";
    static constexpr std::string_view token_equal = "==";
    static constexpr std::string_view token_not_equal = "!=";
    static constexpr std::string_view token_less = "<";
    static constexpr std::string_view token_greater = ">";
    static constexpr std::string_view token_less_equal = "<=";
    static constexpr std::string_view token_greater_equal = ">=";
    static constexpr std::string_view token_assign = "=";
    static constexpr std::string_view token_address = "@";
    static constexpr std::string_view token_dereference = "*";
    static constexpr std::string_view token_drill = "->";
    static constexpr std::string_view token_increment = "++";
    static constexpr std::string_view token_decrement = "--";
    static constexpr std::string_view token_compound_plus = "+=";
    static constexpr std::string_view token_compound_minus = "-=";
    static constexpr std::string_view token_compound_multiply = "*=";
    static constexpr std::string_view token_compound_divice = "/=";
    static constexpr std::string_view token_compound_modulo = "%=";
    static constexpr std::string_view token_compound_bit_and = "&=";
    static constexpr std::string_view token_compound_bit_or = "|=";
    static constexpr std::string_view token_compound_bit_xor = "^=";
    static constexpr std::string_view token_compound_bit_lshift = "<<=";
    static constexpr std::string_view token_compound_bit_rshift = ">>=";

    static bool is_whitespace(char32 c) {
        return (c <= 32) | (c == 127);
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

        bool match(std::string_view const string) {
            ignore_whitespace_and_comments();

            Lexer_State const state_backup = get_current_state();
            for(char c: string) {
                if(get_next() != c) {
                    restore_state(state_backup);
                    return false;
                }
            }
            return true;
        }

        // TODO: String interning if it becomes too slow/memory heavy.
        bool match_identifier(std::string& out) {
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

        bool match_eof() {
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

    private:
        std::istream& _stream;
        i64 _line = 0;
        i64 _column = 0;
    };

    class Parser {
    public:
        Parser(std::istream& stream): _lexer(stream) {}

        bool build_ast() {
            while(!_lexer.match_eof()) {
                if(Declaration* declaration = try_declaration(); declaration) {
                    declaration->print(std::cout, Indent{0});
                    delete declaration;
                } else {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] Parse_Error get_last_error() const {
            return _last_error;
        }

    private:
        Lexer _lexer;
        Parse_Error _last_error;

        void set_error(std::string_view const message, Lexer_State const state) {
            if(state.stream_offset > _last_error.file_offset) {
                _last_error.message = message;
                _last_error.line = state.line;
                _last_error.column = state.column;
                _last_error.file_offset = state.stream_offset;
            }
        }

        void set_error(std::string_view const message) {
            Lexer_State const state = _lexer.get_current_state();
            if(state.stream_offset > _last_error.file_offset) {
                _last_error.message = message;
                _last_error.line = state.line;
                _last_error.column = state.column;
                _last_error.file_offset = state.stream_offset;
            }
        }
        
        Declaration* try_declaration() {
            if(Variable_Declaration* variable_declaration = try_variable_declaration(); variable_declaration) {
                return variable_declaration;
            }

            if(Function_Declaration* function_declaration = try_function_declaration(); function_declaration) {
                return function_declaration;
            }

            return nullptr;
        }

        Variable_Declaration* try_variable_declaration() {
            Lexer_State const state_backup = _lexer.get_current_state();
            if(!_lexer.match(kw_var)) {
                set_error("Expected keyword `var`.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Owning_Ptr<Identifier> var_name = nullptr;
            if(std::string identifier; _lexer.match_identifier(identifier)) {
                var_name = new Identifier(std::move(identifier));
            } else {
                set_error("Expected variable name.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(!_lexer.match(token_colon)) {
                set_error("Expected `:` after variable name.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Owning_Ptr var_type = try_type();
            if(!var_type) {
                set_error("Expected type.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Owning_Ptr<Expression> initializer = nullptr;
            if(_lexer.match(token_assign)) {
                initializer = try_expression();
                if(!initializer) {
                    _lexer.restore_state(state_backup);
                    return nullptr;
                }
            }

            if(!_lexer.match(token_semicolon)) {
                set_error("Expected `;` after variable declaration.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            return new Variable_Declaration(var_type.release(), var_name.release(), initializer.release());
        }

        Function_Declaration* try_function_declaration() {
            Lexer_State const state_backup = _lexer.get_current_state();
            if(!_lexer.match(kw_fn)) {
                set_error("Expected kewyord `fn`.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Owning_Ptr<Identifier> name = nullptr;
            if(std::string fn_name; _lexer.match_identifier(fn_name)) {
                name = new Identifier(std::move(fn_name));
            } else {
                set_error("Expected function name.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Owning_Ptr param_list = try_function_parameter_list();
            if(!param_list) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(!_lexer.match(token_drill)) {
                set_error("Expected `->`.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Owning_Ptr return_type = try_type();
            if(!return_type) {
                set_error("Expected return type.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Owning_Ptr function_body = try_function_body();
            if(!function_body) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            return new Function_Declaration(name.release(), param_list.release(), return_type.release(), function_body.release());
        }

        Function_Parameter* try_function_parameter() {
            Lexer_State const state_backup = _lexer.get_current_state();
            
            Owning_Ptr<Identifier> identifier = nullptr;
            if(std::string identifier_str; _lexer.match_identifier(identifier_str)) {
                identifier = new Identifier(std::move(identifier_str));
            } else {
                set_error("Expected parameter name.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(!_lexer.match(token_colon)) {
                set_error("Expected `:`.");
                return nullptr;
            }

            Owning_Ptr parameter_type = try_type();
            if(!parameter_type) {
                set_error("Expected parameter type.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            return new Function_Parameter(identifier.release(), parameter_type.release());
        }

        Function_Parameter_List* try_function_parameter_list() {
            Lexer_State const state_backup = _lexer.get_current_state();
            if(!_lexer.match(token_paren_open)) {
                set_error("Expected `(`.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(_lexer.match(token_paren_close)) {
                return new Function_Parameter_List;
            }
            
            // Match parameters.
            Owning_Ptr param_list = new Function_Parameter_List;
            {
                Lexer_State const param_list_backup = _lexer.get_current_state();
                do {
                    if(Function_Parameter* parameter = try_function_parameter(); parameter) {
                        param_list->append_parameter(parameter);
                    } else {
                        _lexer.restore_state(param_list_backup);
                        return nullptr;
                    }
                } while(_lexer.match(token_comma));
            }

            if(!_lexer.match(token_paren_close)) {
                set_error("Expected `)` after function parameter list.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            return param_list.release();
        }

        Function_Body* try_function_body() {
            if(!_lexer.match(token_brace_open)) {
                set_error("Expected `{` at the beginning of function body.");
                return nullptr;
            }

            if(_lexer.match(token_brace_close)) {
                return new Function_Body(nullptr);
            }

            Owning_Ptr statements = try_statement_list();
            if(statements->size() == 0) {
                return nullptr;
            }

            if(!_lexer.match(token_brace_close)) {
                set_error("Expected `}` at the end of the function body.");
            return nullptr;
            }

            return new Function_Body(statements.release());
        }

        Statement_List* try_statement_list() {
            Statement_List* statements = new Statement_List;
            while(true) {
                if(Block_Statement* block_statement = try_block_statement()) {
                    statements->append(block_statement);
                    continue;
                }

                if(If_Statement* if_statement = try_if_statement()) {
                    statements->append(if_statement);
                    continue;
                }

                if(While_Statement* while_statement = try_while_statement()) {
                    statements->append(while_statement);
                    continue;
                }

                if(Do_While_Statement* do_while_statement = try_do_while_statement()) {
                    statements->append(do_while_statement);
                    continue;
                }

                if(Variable_Declaration* decl = try_variable_declaration()) {
                    Declaration_Statement* decl_stmt = new Declaration_Statement(decl);
                    statements->append(decl_stmt);
                    continue;
                }

                if(Expression_Statement* expr_stmt = try_expression_statement()) {
                    statements->append(expr_stmt);
                    continue;
                }

                return statements;
            }
        }

        Type* try_type() {
            if(Template_ID* template_id = try_template_id()) {
                return template_id;
            } else if(Qualified_Type* qualified_type = try_qualified_type()) {
                return qualified_type;
            } else {
                return nullptr;
            }
        }

        Template_ID* try_template_id() {
            Lexer_State const state_backup = _lexer.get_current_state();

            Owning_Ptr qualified_type = try_qualified_type();
            if(!qualified_type) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(!_lexer.match(token_angle_open)) {
                set_error("Expected `<`.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(_lexer.match(token_angle_close)) {
                return new Template_ID(qualified_type.release());
            }

            Owning_Ptr template_id = new Template_ID(qualified_type.release());
            do {
                if(Type* type = try_type()) {
                    template_id->append(type);
                } else {
                    return nullptr;
                }
            } while(_lexer.match(token_comma));

            if(!_lexer.match(token_angle_close)) {
                set_error("Expected `>`.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            return template_id.release();
        }

        Qualified_Type* try_qualified_type() {
            if(std::string name; _lexer.match_identifier(name)) {
                return new Qualified_Type(name);
            } else {
                set_error("Expected identifier.");
                return nullptr;
            }
        }

        Block_Statement* try_block_statement() {
            Lexer_State const state_backup = _lexer.get_current_state();
            if(!_lexer.match(token_brace_open)) {
                set_error("Expected `{` at the start of the block.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(_lexer.match(token_brace_close)) {
                return new Block_Statement(nullptr);
            }

            Owning_Ptr statements = try_statement_list();
            if(statements->size() == 0) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(!_lexer.match(token_brace_close)) {
                set_error("Expected `}` at the end of the block.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            return new Block_Statement(statements.release());
        }

        If_Statement* try_if_statement() {
            Lexer_State const state_backup = _lexer.get_current_state();
            if(!_lexer.match(kw_if)) {
                set_error("Expected `if`.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Owning_Ptr condition = try_expression();
            if(!condition) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Owning_Ptr block = try_block_statement();
            if(!block) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }
            
            return new If_Statement(condition.release(), block.release());
        }

        While_Statement* try_while_statement() {
            Lexer_State const state_backup = _lexer.get_current_state();
            if(!_lexer.match(kw_while)) {
                set_error("Expected `while`.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Owning_Ptr condition = try_expression();
            if(!condition) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Owning_Ptr block = try_block_statement();
            if(!block) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            return new While_Statement(condition.release(), block.release());
        }

        Do_While_Statement* try_do_while_statement() {
            Lexer_State const state_backup = _lexer.get_current_state();
            if(!_lexer.match(kw_do)) {
                set_error("Expected `do`.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Owning_Ptr block = try_block_statement();
            if(!block) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(!_lexer.match(kw_while)) {
                set_error("Expected `while`.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            Owning_Ptr condition = try_expression();
            if(!condition) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(!_lexer.match(token_semicolon)) {
                set_error("Expected `;` after do-while statement.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }
            
            return new Do_While_Statement(condition.release(), block.release());
        }

        Expression_Statement* try_expression_statement() {
            Lexer_State const state_backup = _lexer.get_current_state();
            Owning_Ptr expression = try_expression();
            if(!expression) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(!_lexer.match(token_semicolon)) {
                set_error("Expected `;` at the end of statement.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            return new Expression_Statement(expression.release());
        }

        Expression* try_expression() {
            if(auto* boolean_or = try_boolean_or_expression()) {
                return boolean_or;
            }

            if(Bool_Literal* bool_literal = try_bool_literal()) {
                return bool_literal;
            }

            if(Identifier_Expression* identifier_expression = try_identifier_expression()) {
                return identifier_expression;
            }
            
            return nullptr;
        }

        Expression* try_boolean_or_expression() {
            Lexer_State const state_backup = _lexer.get_current_state();
            Owning_Ptr lhs = try_boolean_and_expression();
            if(!lhs) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(!_lexer.match(token_logic_or)) {
                return lhs.release();
            }

            Owning_Ptr rhs = try_boolean_or_expression();
            if(!rhs) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            return new Boolean_Or_Expression(lhs.release(), rhs.release());
        }

        Expression* try_boolean_and_expression() {
            Lexer_State const state_backup = _lexer.get_current_state();
            Owning_Ptr lhs = try_add_sub_expression();
            if(!lhs) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(!_lexer.match(token_logic_and)) {
                return lhs.release();
            }

            Owning_Ptr rhs = try_boolean_and_expression();
            if(!rhs) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            return new Boolean_And_Expression(lhs.release(), rhs.release());
        }

        Expression* try_add_sub_expression() {
            Lexer_State const state_backup = _lexer.get_current_state();
            Owning_Ptr lhs = try_primary_expression();
            if(!lhs) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            bool const is_add = _lexer.match(token_plus);
            if(!is_add && !_lexer.match(token_minus)) {
                return lhs.release();
            }

            Owning_Ptr rhs = try_add_sub_expression();
            if(!rhs) {
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            return new Add_Sub_Expression(is_add, lhs.release(), rhs.release());
        }

        Expression* try_primary_expression() {
            Lexer_State const state_backup = _lexer.get_current_state();
            if(_lexer.match(token_paren_open)) {
                Owning_Ptr paren_expression = try_expression();
                if(!paren_expression) {
                    _lexer.restore_state(state_backup);
                    return nullptr;
                }

                if(_lexer.match(token_paren_close)) {
                    return paren_expression.release();
                } else {
                    set_error("Expected `)`.");
                    _lexer.restore_state(state_backup);
                    return nullptr;
                }
            }

            if(Integer_Literal* integer_literal = try_integer_literal()) {
                return integer_literal;
            }

            if(Function_Call_Expression* function_call = try_function_call_expression()) {
                return function_call;
            }

            if(Bool_Literal* bool_literal = try_bool_literal()) {
                return bool_literal;
            }

            if(Identifier_Expression* identifier_expression = try_identifier_expression()) {
                return identifier_expression;
            }

            return nullptr;
        }

        Function_Call_Expression* try_function_call_expression() {
            Lexer_State const state_backup = _lexer.get_current_state();
            Owning_Ptr<Identifier> identifier = nullptr;
            if(std::string name; _lexer.match_identifier(name)) {
                identifier = new Identifier(std::move(name));
            } else {
                set_error("Expected function name.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            if(!_lexer.match(token_paren_open)) {
                set_error("Expected `(`.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }
            
            Owning_Ptr arg_list = new Argument_List;
            if(_lexer.match(token_paren_close)) {
                return new Function_Call_Expression(identifier.release(), arg_list.release());
            }

            do {
                if(Expression* expression = try_expression()) {
                    arg_list->append(expression);
                } else {
                    _lexer.restore_state(state_backup);
                    return nullptr;
                }
            } while(_lexer.match(token_comma));

            if(!_lexer.match(token_paren_close)) {
                set_error("Expected `)`.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }

            return new Function_Call_Expression(identifier.release(), arg_list.release());
        }

        Integer_Literal* try_integer_literal() {
            _lexer.ignore_whitespace_and_comments();

            Lexer_State const state_backup = _lexer.get_current_state();

            std::string out;
            char32 next = _lexer.peek_next();
            if(next == '-' || next == '+') {
                out += next;
                _lexer.get_next();
            }

            // TODO: Add handling for leading zeros.
            // TODO: Add hexadecimal and binary literals.

            i64 length = 0;
            while(is_digit(_lexer.peek_next())) {
                out += _lexer.get_next();
                length += 1;
            }

            if(length != 0) {
                return new Integer_Literal(std::move(out));
            } else {
                set_error("Expected more than 0 digits.");
                _lexer.restore_state(state_backup);
                return nullptr;
            }
        }

        Bool_Literal* try_bool_literal() {
            if(_lexer.match(kw_true)) {
                return new Bool_Literal(true);
            } else if(_lexer.match(kw_false)) {
                return new Bool_Literal(false);
            } else {
                set_error("Expected bool literal.");
                return nullptr;
            }
        }

        Identifier_Expression* try_identifier_expression() {
            if(std::string name; _lexer.match_identifier(name)) {
                Identifier* identifier = new Identifier(name);
                return new Identifier_Expression(identifier);
            } else {
                set_error("Expected an identifer.");
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
        if(!parser.build_ast()) {
            Parse_Error error = parser.get_last_error();
            std::cout << path << " (" << error.line << ":" << error.column << ") error: " << error.message << '\n';
        }
    }
}
