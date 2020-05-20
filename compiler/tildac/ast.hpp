#ifndef TILDAC_AST_HPP_INCLUDE
#define TILDAC_AST_HPP_INCLUDE

#include <tildac/types.hpp>
#include <tildac/utility.hpp>

#include <string>
#include <vector>
#include <ostream>

namespace tildac {
    struct Indent {
        i64 indent_count = 0;
    };

    std::ostream& operator<<(std::ostream& stream, Indent indent);

    class Syntax_Tree_Node {
    public:
        virtual ~Syntax_Tree_Node() = default;
        virtual void print(std::ostream& stream, Indent indent) const = 0;
    };

    class Identifier: public Syntax_Tree_Node {
    public:
        Identifier(std::string const& string): _name(string) {}
        Identifier(std::string&& string): _name(std::move(string)) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Identifier:\n";
            stream << Indent{indent.indent_count + 1} << "Name: " << _name << "\n";
        }

    private:
        std::string _name;
    };

    class Expression: public Syntax_Tree_Node {};

    class Identifier_Expression: public Expression {
    public:
        Identifier_Expression(Identifier* identifier): _identifier(identifier) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Identifier_Expression:\n";
            _identifier->print(stream, Indent{indent.indent_count + 1});
        }

    private:
        Owning_Ptr<Identifier> _identifier;
    };

    class Type: public Syntax_Tree_Node {};

    class Qualified_Type: public Type {
    public:
        Qualified_Type(std::string name): _name(name) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Qualified_Type:\n";
            stream << Indent{indent.indent_count + 1} << "Type: " << _name << '\n';
        }

    private:
        std::string _name;
    };

    class Template_ID: public Type {
    public:
        Template_ID(Qualified_Type* qualified_type): _qualified_type(qualified_type) {}

        void append(Type* nested_type) {
            _nested_types.emplace_back(nested_type);
        }

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Template_ID:\n";
            stream << Indent{indent.indent_count + 1} << "Type:\n";
            _qualified_type->print(stream, Indent{indent.indent_count + 2});
            stream << Indent{indent.indent_count + 1} << "Nested Types:\n";
            for(auto& nested_type: _nested_types) {
                nested_type->print(stream, Indent{indent.indent_count + 2});
            }
        }

    private:
        std::vector<Owning_Ptr<Type>> _nested_types;
        Owning_Ptr<Qualified_Type> _qualified_type;
    };

    class Declaration: public Syntax_Tree_Node {};

    class Declaration_Sequence: public Syntax_Tree_Node {
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

    class Variable_Declaration: public Declaration {
    public:
        Variable_Declaration(Type* type, Identifier* identifier, Expression* initializer)
        : _Type(type), _identifier(identifier), _initializer(initializer) {}

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

    class Statement;

    class Statement_List: public Syntax_Tree_Node {
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

    class Statement: public Syntax_Tree_Node {};

    class Block_Statement: public Statement {
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

    class If_Statement: public Statement {
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

    class While_Statement: public Statement {
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

    class Declaration_Statement: public Statement {
    public:
        Declaration_Statement(Variable_Declaration* var_decl): _var_decl(var_decl) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Declaration_Statement (Variable Declaration):\n";
            _var_decl->print(stream, Indent{indent.indent_count + 1});
        }

    private:
        Owning_Ptr<Variable_Declaration> _var_decl;
    };

    class Expression_Statement: public Statement {
    public:
        Expression_Statement(Expression* expression): _expr(expression) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Expression_Statement:\n";
            _expr->print(stream, Indent{indent.indent_count + 1});
        }

    private:
        Owning_Ptr<Expression> _expr;
    };

    class Function_Parameter: public Syntax_Tree_Node {
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

    class Function_Parameter_List: public Syntax_Tree_Node {
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

    class Function_Body: public Syntax_Tree_Node {
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

    class Function_Declaration: public Declaration {
    public:
        Function_Declaration(Identifier* name, Function_Parameter_List* function_parameter_list, Type* return_type, Function_Body* body): 
            _name(name), _parameter_list(function_parameter_list), _return_type(return_type), _body(body) {}

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

#endif // !TILDAC_AST_HPP_INCLUDE
