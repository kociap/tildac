#pragma once

#include <tildac/types.hpp>
#include <tildac/utility.hpp>

#include <ostream>
#include <string>
#include <vector>

namespace tildac {
    enum struct AST_Node_Type {
        identifier,
        qualified_type,
        template_id,
        identifier_expression,
        boolean_or_expression,
        boolean_and_expression,
        add_sub_expression,
        mul_div_expression,
        argument_list,
        function_call_expression,
        bool_literal,
        integer_literal,
        declaration_sequence,
        variable_declaration,
        statement_list,
        block_statement,
        if_statement,
        while_statement,
        do_while_statement,
        declaration_statement,
        expression_statement,
        function_parameter,
        function_parameter_list,
        function_body,
        function_declaration,
    };

    struct Source_Info {
        char const* file;
        i64 file_offset;
        i64 line;
        i64 column;
    };

    struct AST_Node {
        Source_Info source_info;
        AST_Node_Type node_type;

        AST_Node(Source_Info source_info, AST_Node_Type type): source_info(source_info), node_type(type) {}
        virtual ~AST_Node() = default;
    };

    struct Identifier: public AST_Node {
        std::string name;

        Identifier(std::string const& string): AST_Node({}, AST_Node_Type::identifier), name(string) {}
        Identifier(std::string&& string): AST_Node({}, AST_Node_Type::identifier), name(std::move(string)) {}
    };

    struct Type: public AST_Node {
        using AST_Node::AST_Node;
    };

    struct Qualified_Type: public Type {
        std::string name;

        Qualified_Type(std::string name): Type({}, AST_Node_Type::qualified_type), name(name) {}
    };

    struct Template_ID: public Type {
        std::vector<Owning_Ptr<Type>> nested_types;
        Owning_Ptr<Qualified_Type> qualified_type;

        Template_ID(Qualified_Type* qualified_type): Type({}, AST_Node_Type::template_id), qualified_type(qualified_type) {}

        void append(Type* nested_type) {
            nested_types.emplace_back(nested_type);
        }
    };

    struct Expression: public AST_Node {
        using AST_Node::AST_Node;
    };

    struct Identifier_Expression: public Expression {
        Owning_Ptr<Identifier> identifier;

        Identifier_Expression(Identifier* identifier): Expression({}, AST_Node_Type::identifier_expression), identifier(identifier) {}
    };

    struct Boolean_Or_Expression: public Expression {
        Owning_Ptr<Expression> lhs;
        Owning_Ptr<Expression> rhs;

        Boolean_Or_Expression(Expression* lhs, Expression* rhs): Expression({}, AST_Node_Type::boolean_or_expression), lhs(lhs), rhs(rhs) {}
    };

    struct Boolean_And_Expression: public Expression {
    public:
        Boolean_And_Expression(Expression* lhs, Expression* rhs): _lhs(lhs), _rhs(rhs) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Boolean_And_Expression:\n";
            _lhs->print(stream, Indent{indent.indent_count + 1});
            _rhs->print(stream, Indent{indent.indent_count + 1});
        }

    private:
        Owning_Ptr<Expression> _lhs;
        Owning_Ptr<Expression> _rhs;
    };

    struct Add_Sub_Expression: public Expression {
    public:
        Add_Sub_Expression(bool is_add, Expression* lhs, Expression* rhs): _lhs(lhs), _rhs(rhs), _is_add(is_add) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Add_Sub_Expression (" << (_is_add ? "addition" : "subtraction") << "):\n";
            _lhs->print(stream, Indent{indent.indent_count + 1});
            _rhs->print(stream, Indent{indent.indent_count + 1});
        }

    private:
        Owning_Ptr<Expression> _lhs;
        Owning_Ptr<Expression> _rhs;
        bool _is_add;
    };

    struct Mul_Div_Expression: public Expression {
    public:
        Mul_Div_Expression(bool is_mul, Expression* lhs, Expression* rhs): _lhs(lhs), _rhs(rhs), _is_mul(is_mul) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Mul_Div_Expression (" << (_is_mul ? "multiplication" : "division") << "):\n";
            _lhs->print(stream, Indent{indent.indent_count + 1});
            _rhs->print(stream, Indent{indent.indent_count + 1});
        }

    private:
        Owning_Ptr<Expression> _lhs;
        Owning_Ptr<Expression> _rhs;
        bool _is_mul;
    };

    struct Argument_List: public AST_Node {
    public:
        void append(Expression* argument) {
            _arguments.emplace_back(argument);
        }

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Argument_List:\n";
            for(auto& argument: _arguments) {
                argument->print(stream, Indent{indent.indent_count + 1});
            }
        }

    private:
        std::vector<Owning_Ptr<Expression>> _arguments;
    };

    struct Function_Call_Expression: public Expression {
    public:
        Function_Call_Expression(Identifier* identifier, Argument_List* arg_list): _identifier(identifier), _arg_list(arg_list) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Function_Call_Expression:\n";
            _identifier->print(stream, Indent{indent.indent_count + 1});
            _arg_list->print(stream, Indent{indent.indent_count + 1});
        }

    private:
        Owning_Ptr<Identifier> _identifier;
        Owning_Ptr<Argument_List> _arg_list;
    };

    struct Bool_Literal: public Expression {
    public:
        Bool_Literal(bool value): _value(value) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Bool_Literal: " << (_value ? "true" : "false") << "\n";
        }

    private:
        bool _value;
    };

    struct Integer_Literal: public Expression {
    public:
        Integer_Literal(std::string value): _value(value) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Integer_Literal: " << _value << "\n";
        }

    private:
        std::string _value;
    };

    struct Declaration: public AST_Node {};

    struct Declaration_Sequence: public AST_Node {
    public:
        void append(Declaration* const declaration) {
            _decls.emplace_back(declaration);
        }

        [[nodiscard]] i64 size() const {
            return _decls.size();
        }

        virtual void print(std::ostream& stream, Indent const indent) const override {
            for(auto& decl: _decls) {
                decl->print(stream, indent);
            }
        }

    private:
        std::vector<Owning_Ptr<Declaration>> _decls;
    };

    struct Variable_Declaration: public Declaration {
    public:
        Variable_Declaration(Type* type, Identifier* identifier, Expression* initializer): _Type(type), _identifier(identifier), _initializer(initializer) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Variable Declaration:\n";
            _Type->print(stream, {indent.indent_count + 1});
            _identifier->print(stream, {indent.indent_count + 1});
            if(_initializer) {
                _initializer->print(stream, {indent.indent_count + 1});
            }
        }

    private:
        Owning_Ptr<Type> _Type = nullptr;
        Owning_Ptr<Identifier> _identifier = nullptr;
        Owning_Ptr<Expression> _initializer = nullptr;
    };

    struct Statement;

    struct Statement_List: public AST_Node {
    public:
        void append(Statement* const declaration) {
            _statements.emplace_back(declaration);
        }

        [[nodiscard]] i64 size() const {
            return _statements.size();
        }

        virtual void print(std::ostream& stream, Indent const indent) const override;

    private:
        std::vector<Owning_Ptr<Statement>> _statements;
    };

    struct Statement: public AST_Node {};

    struct Block_Statement: public Statement {
    public:
        Block_Statement(Statement_List* statements): _statements(statements) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Block_Statement:\n";
            if(_statements) {
                _statements->print(stream, Indent{indent.indent_count + 1});
            }
        }

    private:
        Owning_Ptr<Statement_List> _statements;
    };

    struct If_Statement: public Statement {
    public:
        If_Statement(Expression* condition, Block_Statement* block): _condition(condition), _block(block) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "If_Statement:\n";
            _condition->print(stream, Indent{indent.indent_count + 1});
            _block->print(stream, Indent{indent.indent_count + 1});
        }

    private:
        Owning_Ptr<Expression> _condition;
        Owning_Ptr<Block_Statement> _block;
    };

    struct While_Statement: public Statement {
    public:
        While_Statement(Expression* condition, Block_Statement* block): _condition(condition), _block(block) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "While_Statement:\n";
            _condition->print(stream, Indent{indent.indent_count + 1});
            _block->print(stream, Indent{indent.indent_count + 1});
        }

    private:
        Owning_Ptr<Expression> _condition;
        Owning_Ptr<Block_Statement> _block;
    };

    struct Do_While_Statement: public Statement {
    public:
        Do_While_Statement(Expression* condition, Block_Statement* block): _condition(condition), _block(block) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Do_While_Statement:\n";
            _condition->print(stream, Indent{indent.indent_count + 1});
            _block->print(stream, Indent{indent.indent_count + 1});
        }

    private:
        Owning_Ptr<Expression> _condition;
        Owning_Ptr<Block_Statement> _block;
    };

    struct Declaration_Statement: public Statement {
    public:
        Declaration_Statement(Variable_Declaration* var_decl): _var_decl(var_decl) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Declaration_Statement (Variable Declaration):\n";
            _var_decl->print(stream, Indent{indent.indent_count + 1});
        }

    private:
        Owning_Ptr<Variable_Declaration> _var_decl;
    };

    struct Expression_Statement: public Statement {
    public:
        Expression_Statement(Expression* expression): _expr(expression) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Expression_Statement:\n";
            _expr->print(stream, Indent{indent.indent_count + 1});
        }

    private:
        Owning_Ptr<Expression> _expr;
    };

    struct Function_Parameter: public AST_Node {
    public:
        Function_Parameter(Identifier* identifier, Type* type): _identifier(identifier), _type(type) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Function Parameter:\n";
            _identifier->print(stream, {indent.indent_count + 1});
            _type->print(stream, {indent.indent_count + 1});
        }

    private:
        Identifier* _identifier;
        Type* _type;
    };

    struct Function_Parameter_List: public AST_Node {
    public:
        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Function Parameter List:\n";
            for(Function_Parameter const* const param: _params) {
                param->print(stream, {indent.indent_count + 1});
            }
        }

        void append_parameter(Function_Parameter* parameter) {
            _params.push_back(parameter);
        }

        i64 get_parameter_count() const {
            return _params.size();
        }

    private:
        std::vector<Function_Parameter*> _params;
    };

    struct Function_Body: public AST_Node {
    public:
        Function_Body(Statement_List* statement_list): _statements(statement_list) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Function Body:\n";
            if(_statements) {
                _statements->print(stream, Indent{indent.indent_count + 1});
            }
        }

    private:
        Owning_Ptr<Statement_List> _statements;
    };

    struct Function_Declaration: public Declaration {
    public:
        Function_Declaration(Identifier* name, Function_Parameter_List* function_parameter_list, Type* return_type, Function_Body* body)
            : _name(name), _parameter_list(function_parameter_list), _return_type(return_type), _body(body) {}

        virtual ~Function_Declaration() override {
            delete _name;
            delete _return_type;
            delete _body;
            delete _parameter_list;
        }

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Function Declaration:\n";
            stream << Indent{indent.indent_count + 1} << "Name:\n";
            _name->print(stream, Indent{indent.indent_count + 2});
            stream << Indent{indent.indent_count + 1} << "Parameter List:\n";
            _parameter_list->print(stream, {indent.indent_count + 2});
            stream << Indent{indent.indent_count + 1} << "Return_Type:\n";
            _return_type->print(stream, Indent{indent.indent_count + 2});
            stream << Indent{indent.indent_count + 1} << "Body:\n";
            _body->print(stream, Indent{indent.indent_count + 2});
        }

    private:
        Identifier* _name;
        Function_Parameter_List* _parameter_list;
        Type* _return_type;
        Function_Body* _body;
    };
} // namespace tildac
