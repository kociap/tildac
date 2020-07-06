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
        binary_expression,
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
        return_statement,
        declaration_statement,
        expression_statement,
        function_parameter,
        function_parameter_list,
        function_body,
        function_declaration,
    };

    enum struct Operator {
        binary_or,
        binary_and,
        binary_eq,
        binary_add,
        binary_sub,
        binary_mul,
        binary_div
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

        Identifier(std::string string): AST_Node({}, AST_Node_Type::identifier), name(std::move(string)) {}
    };

    struct Type: public AST_Node {
        using AST_Node::AST_Node;
    };

    struct Qualified_Type: public Type {
        std::string name;

        Qualified_Type(std::string name): Type({}, AST_Node_Type::qualified_type), name(std::move(name)) {}
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

    struct Binary_Expression: public Expression {
        Owning_Ptr<Expression> lhs;
        Operator op;
        Owning_Ptr<Expression> rhs;

        Binary_Expression(Expression* lhs, Operator op, Expression* rhs)
            : Expression({}, AST_Node_Type::binary_expression), lhs(lhs), op(op), rhs(rhs) {}
    };

    struct Argument_List: public AST_Node {
        Argument_List(): AST_Node({}, AST_Node_Type::argument_list) {}

        void append(Expression* argument) {
            arguments.emplace_back(argument);
        }

        std::vector<Owning_Ptr<Expression>> arguments;
    };

    struct Function_Call_Expression: public Expression {
        Owning_Ptr<Identifier> identifier;
        Owning_Ptr<Argument_List> arg_list;

        Function_Call_Expression(Identifier* identifier, Argument_List* arg_list)
            : Expression({}, AST_Node_Type::function_call_expression), identifier(identifier), arg_list(arg_list) {}
    };

    struct Bool_Literal: public Expression {
        bool value;

        Bool_Literal(bool value): Expression({}, AST_Node_Type::bool_literal), value(value) {}
    };

    struct Integer_Literal: public Expression {
        std::string value;

        Integer_Literal(std::string value): Expression({}, AST_Node_Type::integer_literal), value(value) {}
    };

    struct Declaration: public AST_Node {
        using AST_Node::AST_Node;
    };

    struct Declaration_Sequence: public AST_Node {
        std::vector<Owning_Ptr<Declaration>> decls;

        Declaration_Sequence(): AST_Node({}, AST_Node_Type::declaration_sequence) {}

        void append(Declaration* const declaration) {
            decls.emplace_back(declaration);
        }

        [[nodiscard]] i64 size() const {
            return decls.size();
        }
    };

    struct Variable_Declaration: public Declaration {
        Owning_Ptr<Type> type = nullptr;
        Owning_Ptr<Identifier> identifier = nullptr;
        Owning_Ptr<Expression> initializer = nullptr;

        Variable_Declaration(Type* type, Identifier* identifier, Expression* initializer)
            : Declaration({}, AST_Node_Type::variable_declaration), type(type), identifier(identifier), initializer(initializer) {}
    };

    struct Statement;

    struct Statement_List: public AST_Node {
        std::vector<Owning_Ptr<Statement>> statements;

        Statement_List(): AST_Node({}, AST_Node_Type::statement_list) {}

        void append(Statement* const declaration) {
            statements.emplace_back(declaration);
        }

        [[nodiscard]] i64 size() const {
            return statements.size();
        }
    };

    struct Statement: public AST_Node {
        using AST_Node::AST_Node;
    };

    struct Block_Statement: public Statement {
        Owning_Ptr<Statement_List> statements;

        Block_Statement(Statement_List* statements): Statement({}, AST_Node_Type::block_statement), statements(statements) {}
    };

    struct If_Statement: public Statement {
        Owning_Ptr<Expression> condition;
        Owning_Ptr<Block_Statement> block;
        Owning_Ptr<Block_Statement> else_block;
        Owning_Ptr<If_Statement> else_if;

        If_Statement(Expression* condition, Block_Statement* block, Block_Statement* else_block, If_Statement* else_if)
            : Statement({}, AST_Node_Type::if_statement), condition(condition), block(block), else_block(else_block), else_if(else_if) {}
    };

    struct While_Statement: public Statement {
        Owning_Ptr<Expression> condition;
        Owning_Ptr<Block_Statement> block;

        While_Statement(Expression* condition, Block_Statement* block): Statement({}, AST_Node_Type::while_statement), condition(condition), block(block) {}
    };

    struct Do_While_Statement: public Statement {
        Owning_Ptr<Expression> condition;
        Owning_Ptr<Block_Statement> block;

        Do_While_Statement(Expression* condition, Block_Statement* block)
            : Statement({}, AST_Node_Type::do_while_statement), condition(condition), block(block) {}
    };

    struct Return_Statement: public Statement {
        Owning_Ptr<Expression> expression;

        Return_Statement(Expression* expression): Statement({}, AST_Node_Type::return_statement), expression(expression) {}
    };

    struct Declaration_Statement: public Statement {
        Owning_Ptr<Variable_Declaration> var_decl;

        Declaration_Statement(Variable_Declaration* var_decl): Statement({}, AST_Node_Type::declaration_statement), var_decl(var_decl) {}
    };

    struct Expression_Statement: public Statement {
        Owning_Ptr<Expression> expr;

        Expression_Statement(Expression* expression): Statement({}, AST_Node_Type::expression_statement), expr(expression) {}
    };

    struct Function_Parameter: public AST_Node {
        Owning_Ptr<Identifier> identifier;
        Owning_Ptr<Type> type;

        Function_Parameter(Identifier* identifier, Type* type): AST_Node({}, AST_Node_Type::declaration_statement), identifier(identifier), type(type) {}
    };

    struct Function_Parameter_List: public AST_Node {
        std::vector<Owning_Ptr<Function_Parameter>> params;

        Function_Parameter_List(): AST_Node({}, AST_Node_Type::function_parameter_list) {}

        void append_parameter(Function_Parameter* parameter) {
            params.push_back(parameter);
        }

        i64 get_parameter_count() const {
            return params.size();
        }
    };

    struct Function_Body: public AST_Node {
        Owning_Ptr<Statement_List> statements;

        Function_Body(Statement_List* statement_list): AST_Node({}, AST_Node_Type::function_body), statements(statement_list) {}
    };

    struct Function_Declaration: public Declaration {
        Owning_Ptr<Identifier> name;
        Owning_Ptr<Function_Parameter_List> parameter_list;
        Owning_Ptr<Type> return_type;
        Owning_Ptr<Function_Body> body;

        Function_Declaration(Identifier* name, Function_Parameter_List* function_parameter_list, Type* return_type, Function_Body* body)
            : Declaration({}, AST_Node_Type::function_declaration), name(name), parameter_list(function_parameter_list), return_type(return_type), body(body) {}
    };
} // namespace tildac
