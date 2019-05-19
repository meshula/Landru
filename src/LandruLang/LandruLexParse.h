#pragma once

#include <memory>
#include <string>
#include <vector>

namespace llp
{
    enum class Scope
    {
        machine, require, declaration,
        state, statement, function, assignment, expression, type, keyword, literal
    };
    struct AST
    {
        Scope scope;
        std::string name;
        std::vector<std::string> values;
        std::vector<std::shared_ptr<AST>> children;
    };
}

std::shared_ptr<llp::AST> landru_lex_parse(char const*const in, size_t sz);
void landru_ast_print(std::shared_ptr<llp::AST>);
