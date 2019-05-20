
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

typedef char const* pe_CurrPtr;
typedef char const*const pe_EndPtr;

//#include "LandruCompiler/Parser.h"
#include <vector>
#include <string>

// Actually parses a parameter list.
// Expects lparen (expr)* rparen
//
// returns true on success
// outposVec is filled with a precedence ordered RPN token stream. See discussion.
// eg: 3 + 4 * (2 - 1) yields 3 4 2 1 - * +
bool parseExpression(pe_CurrPtr& curr, pe_EndPtr end, std::vector<std::string>& outposVec);

/* discussion:

    // The precedence ordered token stream can be evaluated as follows in order
    // to create an AST tree that can be recursively evaluated to get a result
    // that agrees with C style expression evaluation.

    ASTNode* popNode = currNode;

    std::vector<ASTNode*> nodeStack; // this is where the things that get parameters go

    ASTNode* paramNode = 0;
    for (auto i = paramList.begin(); i != paramList.end(); ++i)
    {
        const std::string& s = *i;
        const char* strstart = s.c_str();
        const char* strend = strstart + s.size();

        if (s == "(")
        {
            nodeStack.push_back(currNode);
            currNode = new ASTNode(kTokenParameters);
        }
        else if (s == ")")
        {
            paramNode = currNode;
            currNode = nodeStack.back();
            nodeStack.pop_back();
        }
        else if (*(strend - 1) == '#')
        {
            if (!paramNode)
                lcRaiseError("Missing parameters for function", curr, 32);

            std::string funcName;
            funcName = s.substr(0, s.size() - 1);
            ASTNode* functionNode = new ASTNode(kTokenFunction, funcName.c_str());
            functionNode->addChild(paramNode);
            paramNode = 0;
            currNode->addChild(functionNode);
        }
        else if (peekIsLiteral(strstart, strend))
            parseLiteral(strstart, strend, currNode, kTokenNullLiteral);
        else if (s == "*")
            currNode->addChild(new ASTNode(kTokenOpMultiply));
        else if (s == "+")
            currNode->addChild(new ASTNode(kTokenOpAdd));
        else if (s == "/")
            currNode->addChild(new ASTNode(kTokenOpDivide));
        else if (s == "-")
            currNode->addChild(new ASTNode(kTokenOpSubtract));
        else if (s == "!")
            currNode->addChild(new ASTNode(kTokenOpNegate));
        else if (s == "%")
            currNode->addChild(new ASTNode(kTokenOpModulus));
        else if (s == ">")
            currNode->addChild(new ASTNode(kTokenOpGreaterThan));
        else if (s == "<")
            currNode->addChild(new ASTNode(kTokenOpLessThan));
        //else if (s == "=")
        //    pushAssign();
        else if (s[0] == '@')
            currNode->addChild(new ASTNode(kTokenGetVariableReference, strstart+1));
        else
            currNode->addChild(new ASTNode(kTokenGetVariable, strstart));
    }
    if (!nodeStack.empty() || !paramNode)
        lcRaiseError("Unmatched parenthesis in expression", curr, 32);

    currNode = popNode;
    currNode->addChild(paramNode);

*/
