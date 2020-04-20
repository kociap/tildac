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
        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Node: Type Name\n";
        }
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

    class Function_Body: public Syntax_Tree_Node {
    public:
        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Node: Function Body\n";
        }
    };

    class Function_Declaration: public Declaration {
    public:
        Function_Declaration(Identifier* name, Type_Name* return_type, Function_Body* body): 
            _name(name), _return_type(return_type), _body(body) {}

        virtual ~Function_Declaration() override {
            delete _name;
            delete _return_type;
            delete _body;
        }

        virtual void print(std::ostream& stream, Indent const indent) const override {
            stream << indent << "Node: Function Declaration\n";
            stream << Indent{indent.indent_count + 1} << "Name:\n";
            _name->print(stream, Indent{indent.indent_count + 2});
            stream << Indent{indent.indent_count + 1} << "Return_Type:\n";
            _return_type->print(stream, Indent{indent.indent_count + 2});
            stream << Indent{indent.indent_count + 1} << "Body:\n";
            _body->print(stream, Indent{indent.indent_count + 2});
        }

    private:
        Identifier* _name;
        Type_Name* _return_type;
        Function_Body* _body;
    };
} // namespace tildac

#endif // !TILDAC_AST_HPP_INCLUDE
