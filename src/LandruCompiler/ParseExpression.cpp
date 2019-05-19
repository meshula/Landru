
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

//
//  ParseExpression.cpp
//
//  Adapted from http://en.wikipedia.org/wiki/Shunting-yard_algorithm
//

#include "ParseExpression.h"
#include "LabText/LabText.h"

#include <string.h>
#include <stdio.h>
#include <vector>
#include <string>

bool pe_more(pe_CurrPtr& curr, pe_EndPtr end)
{
    if (curr >= end)
        return false;

    curr = tsSkipCommentsAndWhitespace(curr, end);
    return (curr != end);
}

char pe_peekChar(pe_CurrPtr& curr, pe_EndPtr end)
{
    pe_more(curr, end);
    return *curr;
}

bool pe_peekChar(char c, pe_CurrPtr& curr, pe_EndPtr end)
{
    pe_more(curr, end);
    return c == *curr;
}

char pe_getChar(pe_CurrPtr& curr, pe_EndPtr end)
{
    pe_more(curr, end);
    char ret = *curr;
    ++curr;
    return ret;
}

// advances cursor past a token
int pe_getToken(pe_CurrPtr& curr, pe_EndPtr end)
{
    pe_more(curr, end);
    char const* tokenStr;
    uint32_t length;
    const char* next = tsGetNameSpacedTokenAlphaNumeric(curr, end, '.', &tokenStr, &length);
    curr = next;
    return next - curr;
}

void pe_getNameSpacedToken(pe_CurrPtr& curr, pe_EndPtr end)
{
    pe_more(curr, end);

    char const* tokenStr;
    uint32_t length;
    const char* next = tsGetNameSpacedTokenAlphaNumeric(curr, end, '.', &tokenStr, &length);

    if (next == end)
        return;

    curr = next;
}

int pe_literalLength(pe_CurrPtr curr, pe_EndPtr end) 
{
    char const* tokenStr;
    uint32_t length;
    char c = pe_peekChar(curr, end);
    bool gotString = c == '"';
    bool numeric = tsIsNumeric(c) || c == '-';
    bool rangedRandom = c == '<';
    bool hex = c == '#';

    const char* start = curr;

    if (gotString) {
        // parsing a string
        curr = tsGetString(curr, end, true, &tokenStr, &length);
    }
    else if (numeric) {
        float floatVal;
        curr = tsGetFloat(curr, end, &floatVal);
    }
    else if (rangedRandom) {
        float floatVal;
        pe_getChar(curr, end); // consume '<'
        pe_more(curr, end);
        curr = tsGetFloat(curr, end, &floatVal);
        pe_more(curr, end);
        if (curr[0] != ',')
            return 0;

        float maxFloatVal;
        pe_getChar(curr, end); // consume ','
        pe_more(curr, end);
        curr = tsGetFloat(curr, end, &maxFloatVal);
        if (pe_peekChar(curr, end) != '>')
            return 0;

        pe_getChar(curr, end); // consume '>'
    }
    else if (hex) {
        unsigned int hexVal = 0;
        pe_getChar(curr, end); // consume '#'
        curr = tsGetHex(curr, end, &hexVal);
    }
    else
    {
        pe_getToken(curr, end);

        // if a parenthesis follows a token, then it's a function, not a literal
        pe_CurrPtr check = curr;
        pe_more(curr, end);
        if (check[0] == '(')
            return 0;
    }

    ptrdiff_t len = curr - start;
    pe_more(curr, end);
    return (int)len;
}

// operators
// precedence   operators       associativity
// 4            !               right to left
// 3            * / %           left to right
// 2            + -             left to right
// 1            =               right to left
//

// precedences from http://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B
// op_preced higher value is higher precedence.
// wikipedia page lower value is higher precendence.

int op_preced(const char c)
{
    int ret;
    // use the wikipedia values (where low values indicate high precendence) in the switch for clarity
    switch(c)    {
        case '!':                           ret = 3; break; // also unary plus and minus
        case '*':  case '/': case '%':      ret = 5; break;
        case '+': case '-':                 ret = 6; break;
        // case << >>                       ret = 7; break;
        case '<': case '>':                 ret = 8; break; // also <= >=
        // case == != :                     ret = 9; break;
        // case && :                        ret = 13; break;
        // case || :                        ret = 14; break;
        case '=':                           ret = 16; break;
    }
    return 20 - ret;    // reverse Wikipedia sense of "higher". Use 20 because the wikipedia page goes up to 18
}

bool op_left_assoc(const char c)
{
    switch(c)    {
        // right to left
        // case unary - and unary +
        case '=': case '!':
            return false;
            
        // left to right
        default:
        case '*': case '/': case '%': case '+': case '-':
            return true;
    }
    return false;
}

unsigned int op_arg_count(const char c)
{
    switch(c)  {
        case '!':
        // case unary - and unary +
            return 1;
            
        case '*': case '/': case '%': case '+': case '-': case '=':
        default:
            return 2;
    }
    return 0;
}

#define is_operator(c)  (c == '+' || c == '-' || c == '/' || c == '*' || c == '!' || c == '%' || c == '=' || c == '<' || c == '>')
#define is_function(c)  ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
#define SCOPE_OPEN_TOKEN '('
#define SCOPE_CLOSE_TOKEN ')'
#define is_scope_open(c) (c == SCOPE_OPEN_TOKEN)
#define is_scope_close(c) (c == SCOPE_CLOSE_TOKEN)

bool shunting_yard(pe_CurrPtr& strpos, pe_EndPtr strend, std::vector<std::string>& outposVec, bool stopOnOuterParen)
{    
    char sc;          // used to record stack element
    
    std::vector<std::string> stackVec;

    size_t stackStopSize = 0;
    if (stopOnOuterParen)
    {
        stackVec.push_back("#");
        stackStopSize = 1;
    }

    bool expectOperator = false;
    
    while(strpos < strend) 
    {
        // read one token from the input stream
        strpos = tsSkipCommentsAndWhitespace(strpos, strend);
        char c = *strpos;
        // If the token is a number, or an identifier, then add it to the output queue.
        int literalLen = pe_literalLength(strpos, strend);
        if (!expectOperator && literalLen)
        {
            const char* startpos = strpos;
            std::string s;
            s.assign(startpos, literalLen);
            outposVec.push_back(s);
            strpos += literalLen - 1; // note: strpos will be incremented once more, hence -1 here 

            // expectOperator is used to force x-3 becomes x 3 - instead of x -3
            // and x+3 becomes x 3 + not x +3
            expectOperator = true;
        }
        // If the token is a function token, variable, or variable reference, then push it onto the stack.
        else if(is_function(c) || c == '@')
        {
			std::string s;
			if (c == '@')
				++strpos;

            const char* startpos = strpos;
            pe_getNameSpacedToken(strpos, strend);
            s.assign(startpos, strpos - startpos);
			if (c == '@')
				s = "@" + s;

            strpos = tsSkipCommentsAndWhitespace(strpos, strend);
            if (is_scope_open(*strpos))
                stackVec.push_back(s+"#");  // if a function push it on the stack
            else
                outposVec.push_back(s); // otherwise, it's a variable, push it on the output queue
            --strpos; // anticipate the increment
            expectOperator = true; // an operator should follow a variable or function
        }
        // If the token is a function argument separator (e.g., a comma):
        else if (c == ',')
        {
            bool pe = false;
            while (stackVec.size() > 0)
            {
                sc = stackVec.back()[0];
                if (is_scope_open(sc))
                {
                    pe = true;
                    break;
                }
                else
                {
                    // Until the token at the top of the stack is a left parenthesis,
                    // pop operators off the stack onto the output queue.
                    outposVec.push_back(stackVec.back());
                    stackVec.pop_back();
                }
            }
            // If no left parentheses are encountered, either the separator was misplaced
            // or parentheses were mismatched.
            if(!pe)
            {
                printf("Error: separator or parentheses mismatched\n");
                return false;
            }
            expectOperator = false;
        }
        // If the token is an operator, op1, then:
        else if (is_operator(c))
        {
            while (stackVec.size() > 0)
            {
                sc = stackVec.back()[0];
                // While there is an operator token, op2, at the top of the stack
                // op1 is left-associative and its precedence is less than or equal to that of op2,
                // or op1 has precedence less than that of op2,
                // Let + and ^ be right associative.
                // Correct transformation from 1^2+3 is 12^3+
                // The differing operator priority decides pop / push
                // If 2 operators have equal priority then associativity decides.
                if (is_operator(sc) &&
                   ((op_left_assoc(c) && (op_preced(c) <= op_preced(sc))) ||
                    (op_preced(c) < op_preced(sc))))
                {
                       // Pop op2 off the stack, onto the output queue;
                       outposVec.push_back(stackVec.back());
                       stackVec.pop_back();
                }
                else 
                {
                    break;
                }
            }
            // push op1 onto the stack.
            std::string s;
            s = c;
            stackVec.push_back(s);
            expectOperator = false;
        }
        // If the token is a left parenthesis, then push it onto the stack.
        else if (is_scope_open(c)) 
        {
            expectOperator = false;
            char test = '\0';
            if (stackVec.size() > 0)
                test = *stackVec.back().rbegin() == '#';
         
            if (test) 
            {
                stackVec.push_back("(");
                outposVec.push_back("("); /// @Lab push a mark to indicate the limit of the parameter scope
            }
            else
                stackVec.push_back("{");
        }
        // If the token is a right parenthesis or brace:
        else if (is_scope_close(c))
        {
            expectOperator = false;
            bool pe = false;

            // Until the token at the top of the stack is a left parenthesis,
            // pop operators off the stack onto the output queue
            while (stackVec.size() > stackStopSize) 
            {
                sc = stackVec.back()[0];
                if (is_scope_close(c) && is_scope_open(sc)) 
                {
                    pe = true;
                    outposVec.push_back(")"); /// @Lab push a mark to indicate the other end of the parameter scope
                    break;
                }
                else if (is_scope_close(c) && sc == '{') 
                {
                    pe = true;
                    break;
                }
                else
                {
                    outposVec.push_back(stackVec.back());
                    stackVec.pop_back();
                }
            }
            // If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
            if (!pe)  
            {
                printf("Error: parentheses mismatched\n");
                return false;
            }
            // Pop the left parenthesis from the stack, but not onto the output queue.
            stackVec.pop_back();
            
            // If the token at the top of the stack is a function token, pop it onto the output queue.
            if (stackVec.size() > stackStopSize) 
            {
                sc = stackVec.back()[0];
                if (is_function(sc))
                {
                    outposVec.push_back(stackVec.back());
                    stackVec.pop_back();
                }
            }
            else if (stopOnOuterParen)
                break;
        }
        else if (c == '"')
        {
            expectOperator = true;
            uint32_t length;
            const char* value;
            strpos = tsGetString(strpos, strend, true, &value, &length);
            std::string s;
            s.assign(value, length);
            outposVec.push_back(s);
            --strpos;   // because an increment follows
        }
        else
        {
            printf("Unknown token: %c found in expression\n", c);
            return false; // Unknown token
        }
        ++strpos;
    } // while !strend
    
    // When there are no more tokens to read:
    // While there are still operator tokens in the stack:
    while(stackVec.size() > stackStopSize)
    {
        sc = stackVec.back()[0];
        if(sc == '(' || sc == ')')
        {
            printf("Error: parentheses mismatched\n");
            return false;
        }
        outposVec.push_back(stackVec.back());
        stackVec.pop_back();
    }
    return true;
}


bool parseExpression(pe_CurrPtr& curr, pe_EndPtr end, std::vector<std::string>& outposVec)
{
    bool result = shunting_yard(curr, end, outposVec, true);
    if (!result)
    {
        return false;
        printf("\nInvalid input\n");
    }
    return true;
}

