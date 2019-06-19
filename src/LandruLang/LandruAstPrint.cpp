
#include "Landru/Landru.h"
#include "LandruLexParse.h"
#include "LandruAST.h"
#include <iostream>

namespace {
    using llp::Scope;
    using llp::AST;
    using std::cout;

    std::string scope_string(Scope s)
    {
        switch (s)
        {
        case Scope::declaration: return "declaration";
        case Scope::require: return "require";
        case Scope::machine: return "machine";
        case Scope::state: return "state";
        case Scope::statement: return "statement";
        case Scope::function: return "function";
        case Scope::assignment: return "assignment";
        case Scope::type: return "type";
        case Scope::keyword: return "keyword";
        case Scope::literal: return "literal";
        default: return "?";
        }
    }

    void print_AST(std::shared_ptr<llp::AST> ast, int indent)
    {
        std::string pad(indent, ' ');
        std::string pad2(indent + 4, ' ');
        cout << pad << ast->name << " : " << scope_string(ast->scope) << "\n";
        for (auto& i : ast->values)
            cout << pad2 << "    { " << i << " }\n";

        for (auto& i : ast->children)
            print_AST(i, indent + 4);
    }
}

void landru_ast_print(std::shared_ptr<llp::AST> ast)
{
    print_AST(ast, 0);
}

EXTERNC void landru_ast_print(LandruAST* ast)
{
    if (LandruAST::valid(ast))
        print_AST(ast->ast, 0);
}
