#ifndef TILDAC_AST_HPP_INCLUDE
#define TILDAC_AST_HPP_INCLUDE

#include <tildac/types.hpp>

#include <string>
#include <vector>
#include <ostream>

namespace tildac {
    struct Indent {
        i64 indent_count = 0;
    };

    std::ostream& operator<<(std::ostream& stream, Indent indent) {
        for(i64 i = 0; i < indent.indent_count; ++i) {
            stream << "  ";
        }
        return stream;
    }

    class Syntax_Tree_Node {
    public:
        virtual ~Syntax_Tree_Node() = default;
        virtual void print(std::ostream& stream, Indent indent) const = 0;
    };

    class Expression: public Syntax_Tree_Node {
    public:
        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Node: Expression\n";
        }
    };

    class Identifier: public Syntax_Tree_Node {
    public:
        Identifier(std::string const& string): _name(string) {}
        Identifier(std::string&& string): _name(std::move(string)) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Node: Identifier\n";
            stream << Indent{indent.indent_count + 1} << "Name: " << _name << "\n";
        }

    private:
        std::string _name;
    };

    class Type_Name: public Syntax_Tree_Node {
    public:
        Type_Name(std::string const& name): _name(name) {}
        Type_Name(std::string&& name): _name(std::move(name)) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Node: Type Name\n";
            stream << Indent{indent.indent_count + 1} << "Type: " << _name << '\n';
        }
    
    private:
        std::string _name;
    };

    class Declaration: public Syntax_Tree_Node {};

    class Declaration_Sequence: public Syntax_Tree_Node {
    public:
        virtual ~Declaration_Sequence() override {
            for(Declaration* decl: _decls) {
                delete decl;
            }
        }

        void append_declaration(Declaration* const declaration) {
            _decls.push_back(declaration);
        }

        i64 get_declarations_count() const {
            return _decls.size();
        }

        virtual void print(std::ostream& stream, Indent const indent) const override {
            for(Declaration* decl: _decls) {
                decl->print(stream, indent);
            }
        }

    private:
        std::vector<Declaration*> _decls;
    };

    class Variable_Declaration: public Declaration {
    public:
        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Node: Variable Declaration\n";
            _type_name->print(stream, {indent.indent_count + 1});
            _identifier->print(stream, {indent.indent_count + 1});
            if(_initializer) {
                _initializer->print(stream, {indent.indent_count + 1});
            }
        }

    private:
        Type_Name* _type_name = nullptr;
        Identifier* _identifier = nullptr;
        Expression* _initializer = nullptr;
    };

    class Function_Parameter: public Syntax_Tree_Node {
    public:
        Function_Parameter(Identifier* identifier, Type_Name* type): _identifier(identifier), _type(type) {}

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Node: Function Parameter\n";
            _identifier->print(stream, {indent.indent_count + 1});
            _type->print(stream, {indent.indent_count + 1});
        }

    private:
        Identifier* _identifier;
        Type_Name* _type;
    };

    class Function_Parameter_List: public Syntax_Tree_Node {
    public:
        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Node: Function Parameter List\n";
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
        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Node: Function Body\n";
        }
    };

    class Function_Declaration: public Declaration {
    public:
        Function_Declaration(Identifier* name, Function_Parameter_List* function_parameter_list, Type_Name* return_type, Function_Body* body): 
            _name(name), _parameter_list(function_parameter_list), _return_type(return_type), _body(body) {}

        virtual ~Function_Declaration() override {
            delete _name;
            delete _return_type;
            delete _body;
            delete _parameter_list;
        }

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Node: Function Declaration\n";
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
        Type_Name* _return_type;
        Function_Body* _body;
    };
} // namespace tildac

#endif // !TILDAC_AST_HPP_INCLUDE
