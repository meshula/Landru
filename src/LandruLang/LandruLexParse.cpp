
#include "Landru/Landru.h"

#include "LandruLexParse.h"
#include "LandruAST.h"
#include "ParseExpression.h"
#include <LabText/LabText.h>
#include <functional>
#include <memory>
#include <vector>

using lab::Text::StrView;

namespace llp
{
    struct StrViewP
    {
		StrViewP() = default;
		~StrViewP() = default;

        StrViewP& operator=(const StrViewP& rh)
        {
            valid = rh.valid;
            curr = rh.curr;
            msg = rh.msg;
            ast = rh.ast;
            return *this;
        }

        bool valid = false;
        StrView curr = { nullptr, 0 };
        char const* msg = nullptr;
        std::shared_ptr<AST> ast;
    };

    StrViewP landruProgram(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP l_requireLine(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP l_machine(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP l_state(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP l_statement(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP l_declaration(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP l_functionDesignator(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP l_parameterList(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP l_expression(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP logical_term(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP logical_factor(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP relation(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP rel_op(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP arithmetic_expression(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP add_op(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP sub_op(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP addend(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP subend(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP term(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP mul_op(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP factor(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP l_typeIdentifier(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP l_dottedIdentifier(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP l_identifier(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP l_literal(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP string_literal(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP EQUAL(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP DIGIT(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP ALPHANUMERIC(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP exponent(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP float_number(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP unsigned_int_number(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP int_number(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP LBRAC(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP RBRAC(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP LPAREN(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
    StrViewP RPAREN(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic);
} // namespace llp


EXTERNC LandruAST* landru_lex_parse(char const*const in, size_t sz)
{
    LandruAST* ast = new LandruAST();
    ast->ast = llp::landru_lex_parse(in, sz);
    return ast;
}

EXTERNC void landru_destroy_ast(LandruAST* ast)
{
    if (LandruAST::valid(ast))
    {
        delete ast;
    }
}



namespace llp {

std::shared_ptr<AST> landru_lex_parse(char const*const in, size_t sz)
{
    StrView src{ in, sz };
    std::vector<llp::StrViewP> diagnostic;
    std::shared_ptr<llp::AST> program = std::make_shared<AST>();
    program->scope = llp::Scope::machine;
    program->name = "program";
    llp::StrViewP curr = llp::landruProgram(src, program, diagnostic);
    return program;
}

enum class Keyword
{
    invalid, require, declare, 
    shared,
    machine,
    state, on, kw_goto,
    kw_int, kw_float, kw_string,
    kw_or, kw_and, kw_not,
    kw_eq, kw_ne, kw_lt, kw_gt, kw_lte, kw_gte,
    kw_add, kw_subtract, kw_multiply, kw_divide
};

Keyword ParseKeyword(const StrView& curr)
{
    if (curr == "require") return Keyword::require;
    if (curr == "declare") return Keyword::declare;
    if (curr == "shared")  return Keyword::shared;
    if (curr == "machine") return Keyword::machine;
    if (curr == "state")   return Keyword::state;
    if (curr == "on")      return Keyword::on;
    if (curr == "goto")    return Keyword::kw_goto;
    if (curr == "int")     return Keyword::kw_int;
    if (curr == "float")   return Keyword::kw_float;
    if (curr == "string")  return Keyword::kw_string;
    if (curr == "||")      return Keyword::kw_or;
    if (curr == "&&")      return Keyword::kw_and;
    if (curr == "!")       return Keyword::kw_not;
    if (curr == "<")       return Keyword::kw_lt;
    if (curr == ">")       return Keyword::kw_gt;
    if (curr == "<=")      return Keyword::kw_lte;
    if (curr == ">=")      return Keyword::kw_gte;
    if (curr == "!=")      return Keyword::kw_ne;
    if (curr == "==")      return Keyword::kw_eq;
    if (curr == "*")       return Keyword::kw_multiply;
    if (curr == "+")       return Keyword::kw_add;
    if (curr == "-")       return Keyword::kw_subtract;
    if (curr == "/")       return Keyword::kw_divide;
    return Keyword::invalid;
}

inline StrView ws(const StrView& curr)
{
    return SkipCommentsAndWhitespace(curr);
}

StrViewP ExpectKeyword(StrView curr, Keyword keyword)
{
    StrView start = ws(curr);
    StrView token;
    // for now, this is fine, because none of the keywords can be followed by an operator.
    // in the future, it might be necessary to test for Alphanumerics, then test for the
    // operators as a second step.
    curr = GetTokenAlphaNumeric(curr, token);
    Keyword kw = ParseKeyword(token);
    bool res = kw == keyword;

    std::shared_ptr<AST> ast;
    if (res)
    {
        ast = std::make_shared<AST>();
        ast->scope = Scope::keyword;
        ast->name.assign(token.curr, token.sz);
    }
    return { res, res ? curr : start, res ? "" : "Unexpected keyword", ast };
}

StrViewP ExpectMathKeyword(StrView curr, Keyword keyword)
{
    StrView start = ws(curr);
    StrView token;
    // for now, this is fine, because none of the keywords can be followed by an operator.
    // in the future, it might be necessary to test for Alphanumerics, then test for the
    // operators as a second step.
    curr = GetTokenExt(curr, "+-*/<=>!&|", token);
    Keyword kw = ParseKeyword(token);
    bool res = kw == keyword;

    std::shared_ptr<AST> ast;
    if (res)
    {
        ast = std::make_shared<AST>();
        ast->scope = Scope::keyword;
        ast->name.assign(token.curr, token.sz);
    }
    return { res, res ? curr : start, res ? "" : "Unexpected keyword", ast };
}


StrViewP ExpectZeroOrMore(StrView start, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic, 
    std::function<StrViewP(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)> fn)
{
    StrViewP res;
    do
    {
        res = fn(start, ast, diagnostic);
        if (res.valid)
            start = res.curr;
    } while (res.valid);
    return { true, start, "" };
}

StrViewP diagnose(StrView curr, const char* msg, std::vector<StrViewP>& diagnostic)
{
    StrViewP res{ false, curr, msg };
    diagnostic.push_back(res);
    return res;
}

StrViewP ExpectOneOrMore(StrView start, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic, 
    std::function<StrViewP(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)> fn)
{
    StrViewP res = fn(start, ast, diagnostic);
    if (!res.valid)
        return StrViewP{ false, start, "" };
    return ExpectZeroOrMore(res.curr, ast, diagnostic, fn);
}

StrViewP RequireOneOrMore(StrView start, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic,
    std::function<StrViewP(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)> fn)
{
    StrViewP res = fn(start, ast, diagnostic);
    if (!res.valid)
        return diagnose(start, "Missing", diagnostic);
    return ExpectZeroOrMore(res.curr, ast, diagnostic, fn);
}

bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_digit(char c)
{
    return (c >= '0' && c <= '9');
}



/*
-------------------------------------------------------------------------------
landruProgram
:
    (l_requireLine | l_declaration | l_machine)+
    ;
*/

StrViewP landruProgram(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrViewP res = ExpectOneOrMore(curr, ast, diagnostic, [](StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic) -> StrViewP
    {
        StrView start = curr;
        StrView token;
        curr = ws(curr);
        if (!curr.sz)
            return { false, curr, "" }; // end of file

        GetTokenAlphaNumeric(curr, token);
        Keyword kw = ParseKeyword(token);
        if (kw == Keyword::invalid)
        {
            StrViewP res = l_requireLine(curr, ast, diagnostic);
            if (!res.valid)
                return diagnose(start, "Expected a require, declaration, or machine statement", diagnostic);
            if (res.ast)
                ast->children.push_back(res.ast);
            return res;
        }
        if (kw == Keyword::declare)
        {
            StrViewP res = l_declaration(curr, ast, diagnostic);
            if (!res.valid)
                return diagnose(start, "Malformed declare statement", diagnostic);
            if (res.ast)
                ast->children.push_back(res.ast);
            return res;
        }
        if (kw == Keyword::machine)
        {
            StrViewP res = l_machine(curr, ast, diagnostic);
            if (!res.valid)
                return diagnose(curr, "Malformed machine statement", diagnostic);
            if (res.ast)
                ast->children.push_back(res.ast);
            return res;
        }

        return diagnose(curr, "Unexpected top lovel token", diagnostic);
    });
    res.curr = ws(res.curr);
    if (res.curr.sz > 0)
        return diagnose(res.curr, "Couldn't parse top level element in program", diagnostic);

    return res;
}

/*
-------------------------------------------------------------------------------
l_requireLine :
    l_identifier '=' 'require' string_literal
    ;
*/

StrViewP l_requireLine(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    std::string identifier;
    std::string value;

    StrView start = ws(curr);
    StrView id;
    curr = ws(GetNameSpacedTokenAlphaNumeric(start, '.', id));
    char c = curr.curr[0];
    if (c != '=')
        return { false, start, "assignment operator = not found" };

    identifier = std::string(id.curr, id.sz);

    curr.curr++;
    curr.sz--;

    curr = GetTokenAlphaNumeric(ws(curr), id);
    Keyword kw = ParseKeyword(id);
    if (kw != Keyword::require)
        return { false, start, "only require keyword valid here" };

    StrViewP req = string_literal(curr, ast, diagnostic);
    if (!req.valid)
        return { false, start, "require should be followed by a quoted string literal" };

    ast = std::make_shared<AST>();
    ast->name = identifier;
    ast->scope = Scope::require;
    ast->values.push_back(req.ast->name);
    return { true, req.curr, "", ast };
}

/*
-------------------------------------------------------------------------------
l_machine:
    'machine' l_identifier
    LBRAC
    (l_declaration|l_state)*
    RBRAC;
*/
StrViewP l_machine(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = curr;
    StrView id;
    curr = GetTokenAlphaNumeric(ws(curr), id);
    Keyword kw = ParseKeyword(id);
    if (kw != Keyword::machine)
        return { false, start, "Expected machine keyword" };

    ast = std::make_shared<AST>();
    ast->scope = Scope::machine;

    StrViewP res = l_identifier(curr, ast, diagnostic);
    if (!res.valid)
        return { false, curr, "Expected machine identifier" };
    ast->name = res.ast->name;

    res = LBRAC(res.curr, ast, diagnostic);
    if (!res.valid)
        return { false, start, "Expected open brace after machine keyword" };

    res = ExpectZeroOrMore(res.curr, ast, diagnostic, [](StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic) -> StrViewP
    {
        curr = ws(curr);
        StrViewP res = l_declaration(curr, ast, diagnostic);
        if (res.valid)
        {
            // found optional declaration block
            ast->children.push_back(res.ast);
            return { true, res.curr, "" };
        }
        res = l_state(curr, ast, diagnostic);
        if (res.valid)
        {
            ast->children.push_back(res.ast);
            return { true, res.curr, "" };
        }
        return { false, curr, "" };
    });
    curr = res.curr;
    res = RBRAC(curr, ast, diagnostic);
    if (!res.valid)
        return diagnose(curr, "Expected closing brace for machine", diagnostic);

    return { true, res.curr, "", ast };
}

/*
-------------------------------------------------------------------------------
    l_state:
    'state' l_identifier LBRAC l_statement* RBRAC
    ;
*/

StrViewP l_state(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    curr = ws(curr);
    StrViewP res = ExpectKeyword(curr, Keyword::state);
    if (!res.valid)
        return { false, curr, "Expected state keyword" };

    ast = std::make_shared<AST>();
    ast->scope = Scope::state;

    curr = res.curr;
    res = l_identifier(curr, ast, diagnostic);
    if (!res.valid)
        return diagnose(curr, "Expected state identifier", diagnostic);

    ast->name = res.ast->name;

    curr = res.curr;
    res = LBRAC(curr, ast, diagnostic);
    if (!res.valid)
        return diagnose(curr, "Expected {", diagnostic);

    res = ExpectZeroOrMore(res.curr, ast, diagnostic, [](StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic) -> StrViewP
    {
        StrViewP res = l_statement(curr, ast, diagnostic);
        if (res.valid)
            ast->children.push_back(res.ast);
        return res;
    });

    curr = res.curr;
    res = RBRAC(curr, ast, diagnostic);
    if (!res.valid)
        return diagnose(curr, "Expected }", diagnostic);

    return { true, res.curr, "", ast };
}

/*
-------------------------------------------------------------------------------
    l_statement
    : ('on' l_functionDesignator LBRAC l_statement* RBRAC)
    | ('goto' l_identifier)
    | l_declaration
    | ((l_dottedIdentifier EQUAL)? l_functionDesignator)
;

ast rules
---------
on:
    name: "on"
    scope: statement
    children: condition, statements

goto:
    name: "goto"
    scope: statement
    values: label

declaration:
    -> l_declaration::ast

assignment:
    name: "assign"
    scope: statement
    values: identifier
    children: l_functionDesignator:ast

function:
    -> l_functionDesignator:ast
    scope: statement
*/

StrViewP l_statement(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);

    ast = std::make_shared<AST>();
    ast->scope = Scope::statement;

    StrViewP res = ExpectKeyword(start, Keyword::on);
    if (res.valid)
    {
        ast->name = "on";

        curr = res.curr;
        res = l_functionDesignator(curr, ast, diagnostic);
        if (!res.valid)
        {
            return diagnose(start, "Expected function to follow on statement", diagnostic);
        }
        curr = res.curr;

        ast->children.push_back(res.ast);

        res = LBRAC(curr, ast, diagnostic);
        if (!res.valid)
            return diagnose(curr, "Expected {", diagnostic);

        res = ExpectZeroOrMore(res.curr, ast, diagnostic, [](StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic) -> StrViewP
        {
            StrViewP res = l_statement(curr, ast, diagnostic);
            if (res.valid)
                ast->children.push_back(res.ast);
            return res;
        });

        curr = res.curr;
        res = RBRAC(curr, ast, diagnostic);
        if (!res.valid)
            return diagnose(curr, "Expected }", diagnostic);

        return { true, res.curr, "", ast };
    }
    res = ExpectKeyword(start, Keyword::kw_goto);
    if (res.valid)
    {
        ast->name = "goto";

        curr = res.curr;
        res = l_identifier(curr, ast, diagnostic);
        if (!res.valid)
            return diagnose(curr, "label must follow goto", diagnostic);

        ast->values.push_back(res.ast->name);
        return { true, res.curr, "", ast };
    }
    res = l_declaration(start, ast, diagnostic);
    if (res.valid)
    {
        return { true, res.curr, "", res.ast };
    }
    res = l_dottedIdentifier(start, ast, diagnostic);
    if (!res.valid)
        return { false, start };

    std::string identifier = res.ast->name;

    StrView next = ws(res.curr);
    if (next.curr[0] == '=')
    {
        next.curr++;
        next.sz--;
        res = l_functionDesignator(next, ast, diagnostic);
        if (!res.valid)
            return diagnose(start, "Expected valid assignment", diagnostic);

        ast = std::make_shared<AST>();
        ast->name = identifier;
        ast->scope = Scope::assignment;
        ast->children.push_back(res.ast);

        return { true, res.curr, "", ast };
    }
    res = l_functionDesignator(start, ast, diagnostic);
    if (!res.valid)
        return { false, start };

    return { true, res.curr, "", res.ast };
}

/*
-------------------------------------------------------------------------------
l_declaration
    : 'declare' LBRAC ('shared'? l_typeIdentifier l_dottedIdentifier (EQUAL literal)?)* RBRAC
    ;
*/

StrViewP l_declaration(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = curr;
    StrViewP res = ExpectKeyword(curr, Keyword::declare);
    if (!res.valid)
        return { false, start, "Expected declare keyword" };

    res = LBRAC(res.curr, ast, diagnostic);
    if (!res.valid)
        return { false, curr, "Expected { to begin declaration block" };

    ast = std::make_shared<AST>();
    ast->scope = Scope::declaration;

    res = ExpectZeroOrMore(res.curr, ast, diagnostic, [](StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic) -> StrViewP
    {
        std::shared_ptr<AST> child = std::make_shared<AST>();
        child->scope = Scope::assignment;

        curr = ws(curr);
        StrViewP res = ExpectKeyword(curr, Keyword::shared);
        if (res.valid)
        {
            curr = res.curr;    // consume shared keyword
            child->values.push_back("shared");
        }
        else
            child->values.push_back("local");

        res = l_typeIdentifier(curr, ast, diagnostic);
        if (!res.valid)
            return { false, curr }; // exit, didn't find a declaration

        child->values.push_back(res.ast->name);

        res = l_dottedIdentifier(res.curr, ast, diagnostic);
        if (!res.valid)
            return diagnose(curr, "identifier must follow type", diagnostic);

        child->name = res.ast->name;

        curr = res.curr;
        res = EQUAL(curr, ast, diagnostic);
        if (res.valid)
        {
            res = l_literal(res.curr, ast, diagnostic);
            if (!res.valid)
                return diagnose(curr, "Expected literal to assign", diagnostic);
            curr = res.curr;
            child->values.push_back(res.ast->name);
        }
        else
            child->values.push_back("");

        ast->children.push_back(child);
        return { true, curr, "", ast }; // continue parsing declarations
    });
    if (!res.valid)
        return res;

    res = RBRAC(res.curr, ast, diagnostic);
    if (!res.valid)
        return diagnose(curr, "Expected } to close declaration block", diagnostic);

    return { true, res.curr, "", ast };
}

/*
-------------------------------------------------------------------------------
l_functionDesignator
    : l_dottedIdentifier LPAREN l_parameterList? RPAREN
    ;

ast rules
---------
    name: identifier
    scope: statement
    children: l_parameterList:ast
*/

StrViewP l_functionDesignator(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res = l_dottedIdentifier(start, ast, diagnostic);
    if (!res.valid)
        return { false, start, "Expected function" };

    ast = res.ast;
    ast->scope = Scope::function;

    StrView paren = ws(res.curr);
    res = LPAREN(paren, ast, diagnostic);
    if (!res.valid)
        return { false, res.curr, "Expected open paren" };
    curr = ws(paren);

    if (!curr.sz)
        return diagnose(curr, "Unexepected EOF", diagnostic);

    if (*curr.curr == ')')
    {
        curr.sz--;
        curr.curr++;
        return { true, curr, "", ast };
    }

    res = l_parameterList(paren, ast, diagnostic);
    // valid if empty list, or valid parsing result
    bool valid = ((res.curr.sz > 0) && (res.curr.curr[0] == ')')) || res.valid;
    if (!valid || !res.ast)
        return { false, curr, "Expected parameter list" };

    ast->children = res.ast->children;
    ast->values = res.ast->values;

    res = RPAREN(res.curr, ast, diagnostic);
    if (!res.valid)
        return { false, curr, "Expected close paren after parameter list" };
    return { true, res.curr, "", ast };
}

/*
-------------------------------------------------------------------------------
l_parameterList
    : l_expression (',' l_expression)*
    ;

ast rules
---------
    children: l_expression:ast+
*/

StrViewP l_parameterList(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    if (!start.sz)
        return diagnose(start, "Unexpected eof in parameter list", diagnostic);

    pe_CurrPtr curr_ptr = start.curr;
    pe_EndPtr end_ptr = start.curr + start.sz;
    ast = std::make_shared<AST>();
    ast->scope = Scope::expression;
    if (parseExpression(curr_ptr, end_ptr, ast->values))
    {
        curr.sz = curr.sz - (curr_ptr - curr.curr);
        curr.curr = curr_ptr;
        return { true, curr, "", ast };
    }
    return { false, start, "invalid expression" };





    StrViewP res = l_expression(start, ast, diagnostic);
    if (!res.valid)
        return { false, start };

    ast = res.ast;

    StrViewP check = ExpectZeroOrMore(res.curr, ast, diagnostic, [] (StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic) -> StrViewP
    {
        StrView start = ws(curr);
        if (!start.sz || start.curr[0] != ',')
            return { false, start, "" };
        start.sz--;
        start.curr++;

        StrViewP res = l_expression(start, ast, diagnostic);
        if (!res.valid)
            return { false, start, "expected expression" };

        for (auto& i : res.ast->children)
            ast->children.push_back(i);

        return { true, res.curr, "", ast };
    });
    if (!check.valid)
        return check;

    return { true, check.curr, "", ast };
}

/*
-------------------------------------------------------------------------------
l_expression
    : logical_term ('||' logical_term)*
    ;

ast rules
---------
    if no "||" found, forward logical_term ast
    if || found,
        scope: statement
        name: "||"
        children: logical_term+
*/

StrViewP l_expression(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);

    std::vector<std::string> rpn_tokens;
    pe_CurrPtr curr_ptr = start.curr;
    pe_EndPtr end_ptr = start.curr + start.sz;
    if (parseExpression(curr_ptr, end_ptr, rpn_tokens))
    {
        curr.sz = curr.sz - (curr_ptr - curr.curr);
        curr.curr = curr_ptr;
        return { true, curr, "" };
    }
    return { false, start, "invalid expression" };


    StrViewP res = logical_term(start, ast, diagnostic);
    if (!res.valid)
        return { false, start, "invalid expression" };
    if (!res.curr.sz)
        return res;

    ast = res.ast;
    start = ws(res.curr);
    res = ExpectMathKeyword(start, Keyword::kw_or);
    if (!res.valid)
        return { true, start, "", ast }; // forward logical_term ast

    std::shared_ptr<AST> or_ast = std::make_shared<AST>();
    or_ast->scope = Scope::statement;
    or_ast->name = "||";
    or_ast->children.push_back(ast);

    res = ExpectOneOrMore(start, or_ast, diagnostic, [](StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic) ->StrViewP
    {
        StrView start = ws(curr);
        StrViewP res = ExpectMathKeyword(start, Keyword::kw_or);
        if (!res.valid)
            return { false, start };
        curr = ws(res.curr);
        res = logical_term(curr, ast, diagnostic);
        if (!res.valid)
            return res;

        ast->children.push_back(res.ast);
        return { true, res.curr, "", ast };
    });

    if (!res.valid)
        return res;

    return { true, res.curr, "", or_ast };
}

/*
-------------------------------------------------------------------------------
logical_term
    : logical_factor ('&&' logical_factor)*
    ;
*/
StrViewP logical_term(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res = logical_factor(start, ast, diagnostic);
    if (!res.valid)
        return { false, start, "" };
    if (!res.curr.sz)
        return res;

    start = ws(curr);
    StrViewP check = ExpectMathKeyword(start, Keyword::kw_and);
    if (!check.valid)
        return res;     // forward the logical_factor

    std::shared_ptr<AST> and_ast = std::make_shared<AST>();
    and_ast->scope = Scope::statement;
    and_ast->children.push_back(res.ast);
    and_ast->name = "&&";

    res = ExpectOneOrMore(start, and_ast, diagnostic, [](StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic) ->StrViewP
    {
        StrView start = ws(curr);
        StrViewP res = ExpectMathKeyword(start, Keyword::kw_and);
        if (!res.valid)
            return { false, start }; 
        curr = ws(res.curr);
        res = logical_factor(curr, ast, diagnostic);
        if (!res.valid)
            return diagnose(curr, "expected logical term", diagnostic);

        ast->children.push_back(res.ast);
        return res;
    });
    if (!res.valid)
        return res;
    return { true, res.curr, "", and_ast };
}

/*
-------------------------------------------------------------------------------
logical_factor
    : ('!')? relation
    ;
*/
StrViewP logical_factor(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res = ExpectMathKeyword(start, Keyword::kw_not);
    bool negate = res.valid;
    if (negate)
        start = res.curr;

    res = relation(curr, ast, diagnostic);
    if (!negate || !res.valid)
        return res;

    ast = std::make_shared<AST>();
    ast->name = "!";
    ast->scope = Scope::statement;
    ast->children.push_back(res.ast);
    return { true, res.curr, "", ast };
}

/*
-------------------------------------------------------------------------------
relation
    : arithmetic_expression (rel_op arithmetic_expression)?
    ;
*/
StrViewP relation(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res = arithmetic_expression(start, ast, diagnostic);
    if (!res.valid)
        return { false, start, "invalid expression" };
    if (!res.curr.sz)
        return res;

    curr = ws(res.curr);
    StrViewP check = rel_op(curr, ast, diagnostic);
    if (!check.valid)
        return { true, curr, "", res.ast };

    check.ast->children.push_back(res.ast);
    res = arithmetic_expression(check.curr, ast, diagnostic);
    if (!res.valid)
        return diagnose(check.curr, "expression must follow relational operator", diagnostic);

    check.ast->children.push_back(res.ast);
    return { true, res.curr, "", check.ast };
}


/*
-------------------------------------------------------------------------------
rel_op
    : '<='
    | '<'
    | '>='
    | '>'
    | '=='
    | '!='
    ;
*/
StrViewP rel_op(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res;
    res = ExpectMathKeyword(curr, Keyword::kw_lte);
    if (res.valid)
        return res;
    res = ExpectMathKeyword(curr, Keyword::kw_lt);
    if (res.valid)
        return res;
    res = ExpectMathKeyword(curr, Keyword::kw_gte);
    if (res.valid)
        return res;
    res = ExpectMathKeyword(curr, Keyword::kw_gt);
    if (res.valid)
        return res;
    res = ExpectMathKeyword(curr, Keyword::kw_eq);
    if (res.valid)
        return res;
    res = ExpectMathKeyword(curr, Keyword::kw_ne);
    if (res.valid)
        return res;
    return { false, start, "Expected relational operator" };
}


/*
-------------------------------------------------------------------------------
arithmetic_expression
    : term (add_op term)*
    ;

ast rules
---------
children and values will be one to one. values will be "+" or "-" indicating operator.

*/
StrViewP arithmetic_expression(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res = subend(start, ast, diagnostic);
    if (!res.valid)
        return diagnose(start, "Expected term for arithmetic expression", diagnostic);

    StrViewP check = ExpectMathKeyword(res.curr, Keyword::kw_add);
    if (!check.valid)
        return res;

    ast = std::make_shared<AST>();
    ast->name = "add_op";
    ast->scope = Scope::statement;
    ast->children.push_back(res.ast);

    res = ExpectOneOrMore(res.curr, ast, diagnostic, [](StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic) -> StrViewP
    {
        StrViewP check = add_op(curr, ast, diagnostic);
        if (!check.valid)
            return check;

        curr = ws(check.curr);
        StrViewP termP = subend(curr, ast, diagnostic);
        if (!termP.valid)
            return termP;

        ast->children.push_back(termP.ast);
        return { true, termP.curr, "", ast };
    });
    if (!res.valid)
        return res;

    return { true, res.curr, "", ast };
}

/*
-------------------------------------------------------------------------------
add_op
    : '+'
    ;
*/
StrViewP add_op(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res;
    res = ExpectMathKeyword(curr, Keyword::kw_add);
    if (res.valid)
        return res;
    return { false, start };
}

StrViewP subend(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res = term(start, ast, diagnostic);
    if (!res.valid)
        return diagnose(start, "Expected term for arithmetic expression", diagnostic);

    StrViewP check = ExpectMathKeyword(res.curr, Keyword::kw_subtract);
    if (!check.valid)
        return res;

    ast = std::make_shared<AST>();
    ast->name = "sub_op";
    ast->scope = Scope::statement;
    ast->children.push_back(res.ast);

    res = ExpectOneOrMore(res.curr, ast, diagnostic, [](StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic) -> StrViewP
    {
        StrViewP check = sub_op(curr, ast, diagnostic);
        if (!check.valid)
            return check;

        curr = ws(check.curr);
        StrViewP termP = term(curr, ast, diagnostic);
        if (!termP.valid)
            return termP;

        ast->children.push_back(termP.ast);
        return { true, termP.curr, "", ast };
    });
    if (!res.valid)
        return res;

    return { true, res.curr, "", ast };
}


/*
-------------------------------------------------------------------------------
sub_op
    | '-'
    ;
*/
StrViewP sub_op(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res;
    res = ExpectMathKeyword(curr, Keyword::kw_subtract);
    if (res.valid)
        return res;
    return { false, start };
}

/*
-------------------------------------------------------------------------------
term
    : factor (mul_op factor)*
    ;
*/
StrViewP term(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res = factor(start, ast, diagnostic);
    if (!res.valid)
        return res;

    StrViewP check = mul_op(res.curr, ast, diagnostic);
    if (!check.valid)
        return res;

    res = ExpectOneOrMore(res.curr, ast, diagnostic, [](StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic) -> StrViewP
    {
        bool divide = false;
        StrViewP res = ExpectMathKeyword(curr, Keyword::kw_multiply);
        if (res.valid)
        {
            curr = ws(res.curr);
            ast->values.push_back("*");
        }
        else
        {
            res = ExpectMathKeyword(curr, Keyword::kw_divide);
            if (!res.valid)
                return { false, curr };

            curr = ws(res.curr);
            ast->values.push_back("/");
        }

        StrViewP check = factor(res.curr, ast, diagnostic);
        if (!check.valid)
            return { false, res.curr };

        ast->children.push_back(check.ast);
        return { true, check.curr, "", ast }; // continue
    });
    if (!res.valid)
        return res;

    return { true, res.curr, "", ast };
}

/*
-------------------------------------------------------------------------------
mul_op
    : '*'
    | '/'
    ;
*/
StrViewP mul_op(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res;
    res = ExpectMathKeyword(curr, Keyword::kw_multiply);
    if (res.valid)
        return res;
    res = ExpectMathKeyword(curr, Keyword::kw_divide);
    if (res.valid)
        return res;
    return { false, start, "Expected add operator" };
}

/*
-------------------------------------------------------------------------------
factor :
    l_functionDesignator | l_dottedIdentifier | literal
    | (LPAREN l_expression RPAREN)
    ;
*/
StrViewP factor(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res = l_functionDesignator(start, ast, diagnostic);
    if (res.valid)
        return res;

    res = l_dottedIdentifier(start, ast, diagnostic);
    if (res.valid)
        return res;

    res = l_literal(start, ast, diagnostic);
    if (res.valid)
        return res;

    res = LPAREN(start, ast, diagnostic);
    if (res.valid)
    {
        curr = res.curr;
        res = l_expression(curr, ast, diagnostic);
        if (!res.valid)
            return diagnose(curr, "expected expression to follow left paren", diagnostic);

        curr = res.curr;
        ast = res.ast;
        res = RPAREN(curr, ast, diagnostic);
        if (!res.valid)
            return diagnose(curr, "expected matching paren after expression", diagnostic);

        return { true, res.curr, "", ast };
    }
    return { false, start, "Expected factor" };
}

/*
-------------------------------------------------------------------------------
l_typeIdentifier
    : l_dottedIdentifier
    | ( 'int' | 'float' | 'string')
    ;
*/

StrViewP l_typeIdentifier(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res = l_dottedIdentifier(start, ast, diagnostic);
    if (res.valid)
        return { true, res.curr, "", res.ast };

    res = ExpectKeyword(start, Keyword::kw_int);
    if (res.valid)
    {
        res.ast->scope = Scope::type;
        return { true, res.curr, "", res.ast };
    }
    res = ExpectKeyword(start, Keyword::kw_float);
    if (res.valid)
    {
        res.ast->scope = Scope::type;
        return { true, res.curr, "", res.ast };
    }
    res = ExpectKeyword(start, Keyword::kw_string);
    if (res.valid)
    {
        res.ast->scope = Scope::type;
        return { true, res.curr, "", res.ast };
    }
    return { false, start };
}

/*
-------------------------------------------------------------------------------
l_dottedIdentifier: l_identifier ('.' l_identifier)*;
*/

StrViewP l_dottedIdentifier(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    StrViewP res = l_identifier(start, ast, diagnostic);
    if (!res.valid)
        return { false, start, "Expected identifier" };
    start = ws(res.curr);

    ast = res.ast;

    if (res.curr.curr[0] != '.')
        return { true, start, "", ast };

    StrViewP check = ExpectZeroOrMore(start, ast, diagnostic, [](StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic) -> StrViewP
    {
        if (curr.curr[0] != '.')
            return { false, curr, "expected dot" };

        curr.curr++;
        curr.sz--;
        StrViewP res = l_identifier(curr, ast, diagnostic);
        if (!res.valid)
            return diagnose(curr, "Expected identifier after dot", diagnostic);

        ast->name += ".";
        ast->name += res.ast->name;

        return { true, res.curr, "", ast };
    });
    if (!check.valid)
        return check;

    return { true, check.curr, "", ast };
}

/*
-------------------------------------------------------------------------------
l_identifier: ALPHA (ALPHANUMERIC | '_')* ;
*/
StrViewP l_identifier(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    if (!is_alpha(start.curr[0]))
        return { false, start, "Expected identifier" };
    curr = start;
    curr.curr++;
    curr.sz--;
    while (curr.sz > 0)
    {
        char c = curr.curr[0];
        if (is_alpha(c) || is_digit(c) || c == '_')
        {
            curr.curr++;
            curr.sz--;
        }
        else
            break;
    }

    ast = std::make_shared<AST>();
    ast->name.assign(start.curr, start.sz - curr.sz);
    return { true, curr, "", ast };
}

/*
-------------------------------------------------------------------------------
literal:
    float_number | int_number | string_literal
    ;
*/

StrViewP l_literal(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = curr;
    StrViewP res = float_number(curr, ast, diagnostic);
    if (res.valid)
        return res;
    res = int_number(curr, ast, diagnostic);
    if (res.valid)
        return res;
    res = string_literal(curr, ast, diagnostic);
    if (res.valid)
        return res;
    return { false, start };
}

/*
-------------------------------------------------------------------------------
string_literal
    : '"' ( S_CHARQQ | S_ESCAPE )* '"'
    | '\'' ( S_CHARQ | S_ESCAPE )* '\''
    ;
*/
StrViewP string_literal(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = curr;
    curr = ws(curr);
    if (curr.curr[0] == '\'')
    {
        curr.curr++;
        curr.sz--;
        StrView strend = ScanForCharacter(curr, '\'');
        if (!strend.sz)
            return diagnose(curr, "Unterminated string literal", diagnostic);
        ast = std::make_shared<AST>();
        ast->scope = Scope::literal;
        ast->name.assign(curr.curr, strend.sz - curr.sz);
        return { true, StrView{strend.curr + 1, strend.sz - 1}, "", ast };
    }
    else if (curr.curr[0] == '"')
    {
        curr.curr++;
        curr.sz--;
        StrView strend = ScanForCharacter(curr, '"');
        if (!strend.sz)
            return diagnose(curr, "Unterminated string literal", diagnostic);
        ast = std::make_shared<AST>();
        ast->scope = Scope::literal;
        ast->name.assign(curr.curr, curr.sz - strend.sz);
        return { true, StrView{strend.curr + 1, strend.sz - 1}, "", ast };
    }
    else
        return { false, start, "String literal not found" };
}

#if 0
S_CHARQQ
    : ~ ["\\]
    ;
S_CHARQ
    : ~['\\]
    ;

S_ESCAPE
    : '\\' ('’' | '\'' | '"' | '?' | '\\' | 'a' | 'b' | 'f' | 'n' | 'r' | 't' | 'v')
    ;
#endif

/*
-------------------------------------------------------------------------------
EQUAL : '=' ;
*/

StrViewP EQUAL(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    curr = ws(curr);
    bool res = curr.curr[0] == '=';
    if (res)
    {
        curr.curr++;
        curr.sz--;
        return { true, curr, "" };
    }
    return { false, curr, "Expected =" };
}

#if 0
ALPHA : [A-Za-z] ;
#endif

/*
-------------------------------------------------------------------------------
DIGIT : [0-9] ;
*/
StrViewP DIGIT(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = curr;
    if (start.sz < 1)
        return { false, curr, "Expected digit, found EOF" };

    char c = start.curr[0];
    if (c < '0' || c > '9')
        return { false, curr, "Expected digit" };

    curr.curr++;
    curr.sz--;
    return { true, curr, "" };
}

/*
-------------------------------------------------------------------------------
        ALPHANUMERIC : [A-Za-z0-9]+ ;
*/
StrViewP ALPHANUMERIC(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    if (curr.sz < 1)
        return { false, curr, "Expected alphanumeric, found EOF" };

    char c = curr.curr[0];
    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
    {
        curr.curr++;
        curr.sz--;
        return { true, curr, "" };
    }
    return { true, curr, "" };
}

/*
-------------------------------------------------------------------------------
exponent
    : ('e') ('+' | '-')? DIGIT+
    ;
*/
StrViewP exponent(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    if (curr.sz < 1)
        return { false, curr };

    if (curr.curr[0] == 'e' || curr.curr[0] == 'E')
    {
        curr.sz--;
        curr.curr++;
    }
    else
        return { false, curr };

    StrViewP res = int_number(curr, ast, diagnostic);
    if (!res.valid)
        return res;

    res.ast->name = "e" + res.ast->name;
    return { true, res.curr, "", res.ast };
}

/*
-------------------------------------------------------------------------------
float_number
    : ('+' | '-')?DIGIT+ (('.' DIGIT+ (exponent)?)? | exponent)
    ;
*/
StrViewP float_number(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    if (start.sz < 1)
        return { false, curr, "Expected float, found EOF" };

    StrViewP res = unsigned_int_number(curr, ast, diagnostic);
    if (!res.valid)
        return res;

    ast = res.ast;

    curr = res.curr;
    if (*curr.curr == '.')
    {
        curr.curr++;
        curr.sz--;
        res = unsigned_int_number(curr, ast, diagnostic);
        if (res.valid)
        {
            curr = res.curr;
            ast->name += '.';
            ast->name += res.ast->name;
        }
    }
    res = exponent(curr, ast, diagnostic);
    if (res.valid)
    {
        curr = res.curr;
        ast->name += 'e';
        ast->name += res.ast->name;
    }
    return { true, res.curr, "", ast };
}

/*
-------------------------------------------------------------------------------
int_number
    :
    (('+' | '-')?DIGIT+)
    ;
*/

StrViewP int_number(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    if (start.sz < 1)
        return { false, start };

    ast = std::make_shared<AST>();
    ast->scope = Scope::literal;

    if (start.curr[0] == '+')
    {
        start.curr++;
        start.sz--;
    }
    else if (start.curr[0] == '-')
    {
        start.curr++;
        start.sz--;
        ast->name = "-";
    }
    if (start.sz < 1)
        return { false, curr };

    bool valid = false;
    curr = start;
    while (curr.sz > 0)
    {
        char c = *curr.curr;
        if (c < '0' || c > '9')
            break;

        valid = true;
        ast->name += c;
        curr.sz--;
        curr.curr++;
    }
    if (!valid)
        return { false, start };

    return { true, curr, "", ast };
}


/*
-------------------------------------------------------------------------------
unsigned_int_number
    :
    DIGIT+
    ;
*/

StrViewP unsigned_int_number(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    StrView start = ws(curr);
    if (start.sz < 1)
        return { false, start };

    ast = std::make_shared<AST>();
    ast->scope = Scope::literal;

    bool valid = false;
    curr = start;
    while (curr.sz > 0)
    {
        char c = *curr.curr;
        if (c < '0' || c > '9')
            break;

        valid = true;
        ast->name += c;
        curr.sz--;
        curr.curr++;
    }
    if (!valid)
        return { false, start };

    return { true, curr, "", ast };
}
/*
-------------------------------------------------------------------------------
LBRAC : '{' ;
*/

StrViewP LBRAC(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    curr = ws(curr);
    bool res = curr.curr[0] == '{';
    if (res)
    {
        curr.curr++;
        curr.sz--;
        return { true, curr, "" };
    }
    return { false, curr, "Expected {" };
}

/*
-------------------------------------------------------------------------------
RBRAC : '}' ;
*/

StrViewP RBRAC(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    curr = ws(curr);
    bool res = curr.curr[0] == '}';
    if (res)
    {
        curr.curr++;
        curr.sz--;
        return { true, curr, "" };
    }
    return { false, curr, "Expected }" };
}

/*
-------------------------------------------------------------------------------
LPAREN : '(' ;
*/

StrViewP LPAREN(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    curr = ws(curr);
    bool res = curr.curr[0] == '(';
    if (res)
    {
        curr.curr++;
        curr.sz--;
        return { true, curr, "" };
    }
    return { false, curr, "Expected (" };
}

/*
-------------------------------------------------------------------------------
RPAREN : '}' ;
*/

StrViewP RPAREN(StrView curr, std::shared_ptr<AST> ast, std::vector<llp::StrViewP>& diagnostic)
{
    curr = ws(curr);
    bool res = curr.curr[0] == ')';
    if (res)
    {
        curr.curr++;
        curr.sz--;
        return { true, curr, "" };
    }
    return { false, curr, "Expected )" };
}

}