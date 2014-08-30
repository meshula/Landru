
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruCompiler/LandruCompiler.h"

#include "LabText/TextScanner.h"
#include "LabJson/json.h"
#include "LandruCompiler/Ast.h"
#include "LandruCompiler/lcRaiseError.h"
#include "LandruCompiler/Parser.h"
#include "LandruCompiler/ParseExpression.h"
#include "LabJson/LabJson.h"

#include <iostream>
#include <string>

using namespace Landru;


void parseAssignment(CurrPtr& curr, EndPtr end);
void parseLandruBlock(CurrPtr&, EndPtr);
void parseLandruVar(CurrPtr&, EndPtr);
void parseElse(CurrPtr& curr, EndPtr end);
void parseFor(CurrPtr& curr, EndPtr end);
void parseGlobalVarDecls(CurrPtr& curr, EndPtr end);
void parseGoto(CurrPtr& curr, EndPtr end);
void parseStatement(CurrPtr& curr, EndPtr end);
void parseParamList(CurrPtr& curr, EndPtr end);
void parseParam(CurrPtr& curr, EndPtr end);
void parseStatements(CurrPtr&, EndPtr);
void parseConditional(CurrPtr& curr, EndPtr end);
void parseState(CurrPtr& curr, EndPtr end);
void parseDeclare(CurrPtr& curr, EndPtr end);
void parseParamBlock(CurrPtr& curr, EndPtr end);
void parseLocals(CurrPtr& curr, EndPtr end);
void parseMachine(CurrPtr& curr, EndPtr end);
void parseOn(CurrPtr& curr, EndPtr end);
void parseFunction(CurrPtr& curr, EndPtr end, TokenId);
bool peekIsLiteral(CurrPtr& curr, EndPtr end);
void parseLiteral(CurrPtr& curr, EndPtr end);


namespace Landru
{
    static const char kDefineChar = ':';
    static const char kTerminalChar = ';';
    TokenId tokenize(const char* str, size_t length);
		
	struct Token
	{
		TokenId token;
		const char* name;
	};
	
#ifdef TOKEN_DECL
#undef TOKEN_DECL
#endif
#define TOKEN_DECL(a, b) kToken##a, b,
	
	Token tokens[] =
	{
		#include "LandruCompiler/TokenDefs.h"
	};
	
#undef TOKEN_DECL
	
	TokenId tokenize(const char* str, size_t length)
	{
		for (int i = 0; i < sizeof(tokens)/sizeof(Token); ++i)
		{
            int l = strlen(tokens[i].name);
            if (l != length)
                continue;
			if (!strncmp(str, tokens[i].name, length))
				return tokens[i].token;
		}
		return kTokenUnknown;
	}

    bool more(CurrPtr& curr, EndPtr end)
    {
        if (curr >= end)
            return false;
        
        curr = tsSkipCommentsAndWhitespace(curr, end);
        return (curr != end) && !lcCurrentError();
    }
        
    char peekChar(CurrPtr& curr, EndPtr end)
    {
        more(curr, end);
        return *curr;
    }
    
    bool peekChar(char c, CurrPtr& curr, EndPtr end)
    {
        more(curr, end);
        return c == *curr;
    }
    
    char getChar(CurrPtr& curr, EndPtr end)
    {
        more(curr, end);
        char ret = *curr;
        ++curr;
        return ret;
    }
    
    TokenId getToken(CurrPtr& curr, EndPtr end)
    {
        more(curr, end);
        TokenId tokenId = kTokenUnknown;
        
        char const* tokenStr = curr;
        uint32_t length = 0;
        if (!strncmp(tokenStr, "<=0", 3)) {
            tokenId = kTokenLte0;
            length = 3;
        }
        else if (!strncmp(tokenStr, ">=0", 3)) {
            tokenId = kTokenGte0;
            length = 3;
        }
        else if (!strncmp(tokenStr, ">0", 2)) {
            tokenId = kTokenGt0;
            length = 2;
        }
        else if (!strncmp(tokenStr, "<0", 2)) {
            tokenId = kTokenLt0;
            length = 2;
        }
        else if (!strncmp(tokenStr, "=0", 2)) {
            tokenId = kTokenEq0;
            length = 2;
        }
        else if (!strncmp(tokenStr, "!=0", 3)) {
            tokenId = kTokenNotEq0;
            length = 3;
        }
        else if (!strncmp(tokenStr, "=", 1)) {
            tokenId = kTokenEq;
            length = 1;
        }
        else if (!strncmp(tokenStr, "{", 1)) {
            tokenId = kTokenOpenBrace;
            length = 1;
        }
        else if (!strncmp(tokenStr, "[", 1)) {
            tokenId = kTokenOpenBracket;
            length = 1;
        }
        
        if (tokenId != kTokenUnknown)
        {
            curr += length;
            return tokenId;
        }
        
        const char* next = tsGetTokenAlphaNumeric(curr, end, &tokenStr, &length);
        
        if (next == end)
            return kTokenUnknown;
        
        curr = next;
        
        tokenId = tokenize(tokenStr, length);
        return tokenId;
    }
        
    TokenId peekToken(CurrPtr curr_, EndPtr end)
    {
        char const* curr = curr_;
        return getToken(curr, end);
    }

    bool getToken(TokenId token, CurrPtr& curr, EndPtr end)
    {
        if (peekToken(curr, end) != token)
            return false;
        
        getToken(curr, end);
        return true;
    }
    
    bool getChar(char c, CurrPtr& curr, EndPtr end)
    {
        if (!more(curr, end))
            return false;
        
        if (peekChar(curr, end) != c)
            return false;
        
        getChar(curr, end);
        return true;
    }
    
    
    TokenId getNameSpacedToken(CurrPtr& curr, EndPtr end)
    {
        more(curr, end);
        
        char const* tokenStr;
        uint32_t length;
        const char* next = tsGetNameSpacedTokenAlphaNumeric(curr, end, '.', &tokenStr, &length);
        
        if (next == end)
            return kTokenUnknown;
        
        if (next == curr)
        {
            lcRaiseError("Syntax Error", curr, 32);
            return kTokenUnknown;
        }
        curr = next;
        
        TokenId token = tokenize(tokenStr, length);
        return token;
    }

    /// @TODO - this kills thread safety. It should be passed through the parser as part of a parse context object
    ASTNode* currNode = 0;
} // Landru



// Lexical stuff

bool isRelop1(char const* curr)
{
	char c = curr[0];
	return c == '=' || c == '>' || c == '<';
}

bool isRelop2(char const* curr)
{
	return (isRelop1(curr) || curr[0] == '!') && isRelop1(curr+1);
}

bool isAssignment(char const* curr)
{
	return (curr[0] == '=' && !isRelop1(curr+1));
}



void getDeclarator(CurrPtr& curr, EndPtr end, char* name)
{
	more(curr, end);
	char const* nameStr;
	uint32_t length;
	curr = tsGetTokenAlphaNumeric(curr, end, &nameStr, &length);
	strncpy(name, nameStr, length);
	name[length] = '\0';
}

void getType(CurrPtr& curr, EndPtr end, char* name)
{
	more(curr, end);
	char const* nameStr;
	uint32_t length;
	curr = tsGetNameSpacedTokenAlphaNumeric(curr, end, '.', &nameStr, &length);
	strncpy(name, nameStr, length);
	name[length] = '\0';
}

/*
 declarator
	 :   ID (DOT declarator)*
	 -> ^(TokenName ID+)
	 ;
 */

void getNameSpacedDeclarator(CurrPtr& curr, EndPtr end, char* name)
{
	getType(curr, end, name);
}

bool getColon(CurrPtr& curr, EndPtr end)
{
	bool colon = more(curr, end);
	if (colon)
	{
		colon = *curr == ':';
	}
	if (colon)
	{
		++curr;
		return true;
	}
	lcRaiseError("Expected ':'", curr, 32);
	return false;
}

bool getSemiColon(CurrPtr& curr, EndPtr end)
{
	bool colon = more(curr, end);
	if (colon)
	{
		colon = *curr == ';';
	}
	if (colon)
	{
		++curr;
		return true;
	}
	lcRaiseError("Expected ';'", curr, 32);
	return false;
}


//-----------------------------------------------------------------------
//
// DECLARATION BLOCK
//
//-----------------------------------------------------------------------

/*
 lParamBlock 
 :	
 PARAM COLON 
 landruVar+
 SEMICOLON
 -> ^(PARAM landruVar+)
 ;
 */

void parseParamBlock(CurrPtr& curr, EndPtr end)
{
	if (getToken(curr, end) != kTokenParam)
	{
		lcRaiseError("Expected 'param'", curr, 32);
		return;
	}
	
	if (!getColon(curr, end))
		return;
    
	ASTNode* ast = new ASTNode(kTokenParam, "");
	currNode->addChild(ast);
	ASTNode* pop = currNode;
	currNode = ast;
    
	while (more (curr, end))
	{
		if (*curr == kTerminalChar)
		{
			++curr;
			break;
		}
		
		parseLandruVar(curr, end);
	}
	
	currNode = pop;
}

void parseLocals(CurrPtr& curr, EndPtr end) {
	if (getToken(curr, end) != kTokenLocal)
	{
		lcRaiseError("Expected 'local'", curr, 32);
		return;
	}

	if (!getColon(curr, end))
		return;

	ASTNode* ast = new ASTNode(kTokenLocal, "");
	currNode->addChild(ast);
	ASTNode* pop = currNode;
	currNode = ast;

	while (more (curr, end))
	{
		if (*curr == kTerminalChar)
		{
			++curr;
			break;
		}

		parseLandruVar(curr, end);
	}

	currNode = pop;
}

/*
 lDeclareBlock 
	 :	
	 DECLARE COLON 
	 landruVar+
	 SEMICOLON
	 -> ^(DECLARE landruVar+)
	 ;
 */

void parseDeclare(CurrPtr& curr, EndPtr end)
{
	if (getToken(curr, end) != kTokenDeclare)
	{
		lcRaiseError("Expected 'declare'", curr, 32);
		return;
	}
	
	if (!getColon(curr, end))
		return;

	ASTNode* ast = new ASTNode(kTokenDeclare, "");
	currNode->addChild(ast);
	ASTNode* pop = currNode;
	currNode = ast;

	while (more (curr, end))
	{
		if (*curr == kTerminalChar)
		{
			++curr;
			break;
		}
		
		parseLandruVar(curr, end);
	}
	
	currNode = pop;
}

/* 
 landruVar
	 :   
	 landruType declarator 	
	 -> ^(TokenVariable landruType declarator) |
     SHARED LandruType declarator
	 -> ^(TokenSharedVariable landruType declarator) |
	 ;
 landruType
	 :  
	 ID (DOT landruType)*
	 -> ^(TokenType ID+)
	 ;
 */

void parseLandruVar(CurrPtr& curr, EndPtr end)
{
	bool sharedToken = peekToken(curr, end) == kTokenShared;
	if (sharedToken)
		getToken(curr, end);
	
	char varType[256];
	getType(curr, end, varType);

    if (!strlen(varType))
        lcRaiseError("Expected variable type", curr, 32);
    
	char name[256];
	getDeclarator(curr, end, name);
    
    if (!strlen(name))
        lcRaiseError("Expected variable name", curr, 32);
    
	currNode->addChild(new ASTNode(sharedToken ? kTokenSharedVariable : kTokenLocalVariable,
						           varType, name));
}


//-----------------------------------------------------------------------
//
// STATE BLOCK
//
//-----------------------------------------------------------------------

/*
	lStateBlock 
	:
	STATE declarator COLON 
	landruStatements*
	SEMICOLON
	-> ^(STATE ^(declarator) landruStatements*)
	;
 */

void parseState(CurrPtr& curr, EndPtr end)
{
	if (getToken(curr, end) != kTokenState)
	{
		lcRaiseError("Expected 'state'", curr, 32);
	}

	char name[256];
	getDeclarator(curr, end, name);
	
	if (!getColon(curr, end))
		return;

	ASTNode* pop = currNode;
	ASTNode* ast = new ASTNode(kTokenState, name);
	currNode->addChild(ast);
	currNode = ast;

	// landruStatements*
	parseStatements(curr, end);
	getSemiColon(curr, end);
	
	currNode = pop;
}

/*
 landruStatements* ;
 */

void parseStatements(CurrPtr& curr, EndPtr end)
{
	ASTNode* pop = currNode;
	ASTNode* ast = new ASTNode(kTokenStatements);
	pop->addChild(ast);
	currNode = ast;
	
	while (more(curr, end))
	{
		if (*curr == kTerminalChar) // ; at the end of the block of statements
		{
			break;
		}
		
		parseStatement(curr, end);
	}
	currNode = pop;
}

/*
 landruStatements
	 : (lVariable EQ) => lAssignment 
	 | lConditional
	 | lOn
	 | lProcedural
	 | lGoto
	 | lFunction
	 ; 
 */

void parseStatement(CurrPtr& curr, EndPtr end)
{
	TokenId token = peekToken(curr, end);
	switch (token)
	{
        case kTokenParam:
            parseParamBlock(curr, end);
            break;

        case kTokenLocal:
            parseLocals(curr, end);
            break;

		case kTokenUnknown: {
			CurrPtr orig = curr;
			char buff[256];
			getNameSpacedDeclarator(curr, end, buff);

			if (peekChar(curr, end) == '=') {
				curr = orig;
				parseAssignment(curr, end);
			}
			else {
				curr = orig;
				parseFunction(curr, end, kTokenFunction);
			} }
            break;

		case kTokenTrue: {
			currNode->addChild(new ASTNode(kTokenTrue));
			getToken(curr, end); }
            break;

		case kTokenFalse: {
			currNode->addChild(new ASTNode(kTokenFalse));
			getToken(curr, end); }
            break;

		case kTokenGoto:
			parseGoto(curr, end);
			break;

		case kTokenOn:
			parseOn(curr, end);
			break;
			
        case kTokenFor:
            parseFor(curr, end);
            break;
						
		case kTokenIf:
			parseConditional(curr, end);
			break;
									
		case kTokenLaunch:
            parseFunction(curr, end, kTokenLaunch);
			break;
            
        default:
            lcRaiseError("Expected token", curr, 32);
            break;
	}
}

/*
lGoto	:	
	GOTO ID 
	-> ^(GOTO ID) 
	;
*/

void parseGoto(CurrPtr& curr, EndPtr end) {
	getToken(curr, end);	// goto
	char buff[256];
	getDeclarator(curr, end, buff);
	currNode->addChild(new ASTNode(kTokenGoto, buff));
}


/*
 lAssignment
	:   lVariable EQ NEW LPAREN STRING_LITERAL RPAREN
	-> ^(TokenAssign lVariable ^(NEW STRING_LITERAL))
	;	
 */

// note: this should actually be:
/*
 lAssignment
	:   lVariable EQ lValue
	-> ^(TokenAssign lVariable lValue)
	;	
 */
// but I haven't got lValue ground through Antlr yet

void parseAssignment(CurrPtr& curr, EndPtr end) {
	char buff[256];
	getNameSpacedDeclarator(curr, end, buff);
	
	ASTNode* ast = new ASTNode(kTokenAssignment, buff);
	currNode->addChild(ast);
	ASTNode* pop = currNode;
	currNode = ast;

	char assign = getChar(curr, end);
	if (assign != '=') {
		lcRaiseError("Expected '='", curr, 32);
		return;
	}
	
	// fetch right hand side
	parseParam(curr, end);
	
	currNode = pop;
}

/*
lConditional
	:	
	IF lCondition lParamList? COLON
	landruStatements*
	SEMICOLON
	lElse?
	-> ^(IF ^(lCondition lParamList?) landruStatements* lElse?)
	;
*/

void parseConditional(CurrPtr& curr, EndPtr end) {
	TokenId tokenId = getToken(curr, end);
	if (tokenId != kTokenIf) {
		lcRaiseError("Expected 'if'", curr, 32);
		return;
	}

    // if optional(declarator) (params)
    if (peekChar(curr, end) == '(')
        tokenId = kTokenNotEq0;
    else
        tokenId = getToken(curr, end);
	
	ASTNode* pop = currNode;
	ASTNode* astIf = new ASTNode(kTokenIf);
	currNode->addChild(astIf);
	ASTNode* astQualifier = new ASTNode(tokenId);
	astIf->addChild(astQualifier);
	ASTNode* astParams = new ASTNode(kTokenParameters);
	astQualifier->addChild(astParams);
	currNode = astParams;

	if (peekChar(curr, end) == '(')
		parseParamList(curr, end);
	
	currNode = astIf;
	
	if (!getColon(curr, end))
		return;
	
	parseStatements(curr, end);
	getSemiColon(curr, end);

	tokenId = peekToken(curr, end);
	if (tokenId == kTokenElse)
		parseElse(curr, end);
	
	currNode = pop;
}

/*
lElse
	:
	ELSE COLON landruStatements* SEMICOLON
	-> ^(ELSE landruStatements*)
	;
 */

void parseElse(CurrPtr& curr, EndPtr end) {
	getToken(curr, end);	// consume else

	if (!getColon(curr, end))
		return;
		
	ASTNode* pop = currNode;
	currNode = new ASTNode(kTokenElse);
	pop->addChild(currNode);

	parseStatements(curr, end);
	getSemiColon(curr, end);

	currNode = pop;
}

/*
 lFor
 :
 FOR declarator IN landruFunction COLON
 landruStatements*
 SEMICOLON
 -> ^(FOR ^(lOnType lParamList?) landruStatements*)
 ;
 */

void parseFor(CurrPtr& curr, EndPtr end) {
	TokenId tokenId = getToken(curr, end); // consume for
    if (tokenId != kTokenFor) {
        lcRaiseError("Expected 'for'", "parseFor", 0);
    }
	
	char name[256];
	name[0] = '\0';
	TokenId what = peekToken(curr, end);
	if (what == kTokenUnknown) {
		char const* tokenStr;
		uint32_t length;
		curr = tsGetNameSpacedTokenAlphaNumeric(curr, end, '.', &tokenStr, &length);
		strncpy(name, tokenStr, length);
		name[length] = '\0';
		what = kTokenParameters;
	}
	else {
        lcRaiseError("Expected a placeholder variable name", "parseFor", 0);
	}
    
    ASTNode* pop = currNode;
	ASTNode* forNode = new ASTNode(kTokenFor, name);
	currNode->addChild(forNode);
	currNode = forNode;
    
    tokenId = getToken(curr, end);
    if (tokenId != kTokenIn) {
        lcRaiseError("Expected 'in'", "parseFor", 0);
    }
    
    parseFunction(curr, end, kTokenFunction);
    
	if (!getColon(curr, end)) {
        lcRaiseError("Expected ':'", "parseFor", 0);
		return;
    }
	
	parseStatements(curr, end);
	getSemiColon(curr, end);
    
	currNode = pop;
}

/*
 lOn
	 : 
	 ON lOnType lParamList? COLON
	 landruStatements*
	 SEMICOLON 
	 -> ^(ON ^(lOnType lParamList?) landruStatements*)
	 ;
 fragment lOnType	
	: COLLISION
	| MESSAGE
	| TICK
	;
 */

void parseOn(CurrPtr& curr, EndPtr end)
{
	getToken(curr, end); // consume on
	
	char name[256];
	name[0] = '\0';
	TokenId what = peekToken(curr, end);
	if (what == kTokenUnknown) {
		char const* tokenStr;
		uint32_t length;
		curr = tsGetNameSpacedTokenAlphaNumeric(curr, end, '.', &tokenStr, &length);
		strncpy(name, tokenStr, length);
		name[length] = '\0';
		what = kTokenLibEvent;
	}
	else {
		getToken(curr, end);
	}
    
    ASTNode* pop = currNode;
	ASTNode* onNode = new ASTNode(kTokenOn);
	currNode->addChild(onNode);
	currNode = onNode;
	ASTNode* eventNode = new ASTNode(what, name);
	onNode->addChild(eventNode);
	currNode = eventNode;				// curr is now the qualifier, eg. message

    more(curr, end);
    if (peekChar(curr, end) == '(')
        parseParamList(curr, end);

	currNode = onNode;
		
	if (!getColon(curr, end))
		return;
	
	parseStatements(curr, end);
	getSemiColon(curr, end);

	currNode = pop;
}

/*
 lFunction 
	: declarator lParamList
	-> ^(TokenFunction declarator lParamList) ;
 
 functions can be chained with '.' ie foo().bar().baz()
 */

void parseFunction(CurrPtr& curr, EndPtr end, TokenId token)
{
	char buff[256];
	getNameSpacedDeclarator(curr, end, buff);
	
	ASTNode* pop = currNode;
	currNode = new ASTNode(token, buff);
	pop->addChild(currNode);
	parseParamList(curr, end);
    
    // chained functions assume that the previous function left something on the stack
    // so pop out before parsing chain
	currNode = pop;
    
    more(curr, end);
    if (peekChar(curr, end) == '.') {
        ++curr;
        currNode->addChild(new ASTNode(kTokenDotChain, buff));
        parseFunction(curr, end, kTokenFunction);
    }    
}


/*
 lParamList
 :  	
 LPAREN (lParam (COMMA? lParam)*)? RPAREN 
 -> ^(TokenParams lParam*) ;
 */

void parseParamList(CurrPtr& curr, EndPtr end)
{
	// if parsing a function, then an opening paren must have been encountered
	if (peekChar(curr, end) != '(') {
		lcRaiseError("Missing open parenthesis", curr, 32);
		return;
	}
	
    std::vector<std::string> paramList;
    parseExpression(curr, end, paramList);
    
    // closing paren
    if (peekChar(curr, end) == ')')
        getChar(curr, end);  // consume ')'
	else
		lcRaiseError("Expected closing parenthesis, found", curr, 32);
    
    ASTNode* popNode = currNode;
    
    std::vector<ASTNode*> nodeStack; // this is where the things that get parameters go
    
    ASTNode* paramNode = 0;
    for (auto i = paramList.begin(); i != paramList.end(); ++i) {
        const std::string& s = *i;
        const char* strstart = s.c_str();
        const char* strend = strstart + s.size();

        if (s == "(") {
            nodeStack.push_back(currNode);
            currNode = new ASTNode(kTokenParameters);
        }
        else if (s == ")") {
            paramNode = currNode;
            currNode = nodeStack.back();
            nodeStack.pop_back();
        }
        else if (*(strend - 1) == '#') {
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
            parseLiteral(strstart, strend);
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
        //else if (s == "=")
        //    pushAssign();
        else
            currNode->addChild(new ASTNode(kTokenType, strstart));
    }
    if (!nodeStack.empty() || !paramNode)
        lcRaiseError("Unmatched parenthesis in expression", curr, 32);

    currNode = popNode;
    currNode->addChild(paramNode);
}


/*
 lParam
	:   lFunction
	|   lLiteral
	|   lVariable
	;
 
 lVariable
	:   ID (DOT lVariable)*
	-> ^(TokenName ID+)
	;
 */

void parseParam(CurrPtr& curr, EndPtr end)
{	
	if (peekChar(curr, end) == ')')
		return;
	
	if (peekIsLiteral(curr, end))
	{
		parseLiteral(curr, end);
		return;
	}
	
	char buff[256];
	char const* tokenStr;
	uint32_t length;
	curr = tsGetNameSpacedTokenAlphaNumeric(curr, end, '.', &tokenStr, &length);
	strncpy(buff, tokenStr, length);
	buff[length] = '\0';

	if (peekChar(curr, end) == '(')
	{
		ASTNode* pop = currNode;
		currNode = new ASTNode(kTokenFunction, buff);
		pop->addChild(currNode);
		parseParamList(curr, end);
		currNode = pop;
	}
	else
	{
		currNode->addChild(new ASTNode(kTokenType, buff));
	}
	more(curr, end);
}



/*
 lLiteral
 :   RANGED_LITERAL
 |   HEX_LITERAL
 |   OCTAL_LITERAL
 |   DECIMAL_LITERAL
 |   CHARACTER_LITERAL
 |   STRING_LITERAL
 |   FLOATING_POINT_LITERAL
 |   BOOL_LITERAL
 ;
 */


bool peekIsLiteral(CurrPtr& curr, EndPtr end)
{
	char c = peekChar(curr, end);
	if (c == '"' || tsIsNumeric(c) || c == '#' || c == '-' || c == '<')
		return true;
	
	TokenId token = peekToken(curr, end);
	return token == kTokenFalse || token == kTokenTrue;
}

int literalLength(CurrPtr& curr, EndPtr end)
{
	char const* tokenStr;
	uint32_t length;
	char c = peekChar(curr, end);
	bool gotString = c == '"';
	bool numeric = tsIsNumeric(c) || c == '-';
	bool rangedRandom = c == '<';
	bool hex = c == '#';
    
    const char* start = curr;
	
	if (gotString)
	{
		// parsing a string
		curr = tsGetString(curr, end, true, &tokenStr, &length);
	}
	else if (numeric)
	{
		float floatVal;
		curr = tsGetFloat(curr, end, &floatVal);
	}
	else if (rangedRandom)
	{
		float floatVal;
		getChar(curr, end); // consume '<'
		more(curr, end);
		curr = tsGetFloat(curr, end, &floatVal);
		more(curr, end);
		if (curr[0] != ',')
			lcRaiseError("Syntax Error", curr, 32);
		float maxFloatVal;
		getChar(curr, end); // consume ','
		more(curr, end);
		curr = tsGetFloat(curr, end, &maxFloatVal);
		if (peekChar(curr, end) != '>')
			lcRaiseError("Syntax Error", curr, 32);
		getChar(curr, end); // consume '>'
	}
	else if (hex)
	{
		unsigned int hexVal = 0;
		getChar(curr, end); // consume '#'
		curr = tsGetHex(curr, end, &hexVal);
	}
	else
	{
		getToken(curr, end);
	}
    int len = curr - start;
	more(curr, end);
    return len;
}

void parseLiteral(CurrPtr& curr, EndPtr end)
{
	char buff[256];
	char const* tokenStr;
	uint32_t length;
	char c = peekChar(curr, end);
	bool gotString = c == '"';
	bool numeric = tsIsNumeric(c) || c == '-';
	bool rangedRandom = c == '<';
	bool hex = c == '#';
	
	if (gotString)
	{
		// parsing a string
		curr = tsGetString(curr, end, true, &tokenStr, &length);
		strncpy(buff, tokenStr, length);
		buff[length] = '\0';
		currNode->addChild(new ASTNode(kTokenStringLiteral, buff));
	}
	else if (numeric)
	{
		float floatVal;
		curr = tsGetFloat(curr, end, &floatVal);
		currNode->addChild(new ASTNode(kTokenFloatLiteral, floatVal));
	}
	else if (rangedRandom)
	{
		float floatVal;
		getChar(curr, end); // consume '<'
		more(curr, end);
		curr = tsGetFloat(curr, end, &floatVal);
		more(curr, end);
		if (curr[0] != ',')
			lcRaiseError("Syntax Error", curr, 32);
		float maxFloatVal;
		getChar(curr, end); // consume ','
		more(curr, end);
		curr = tsGetFloat(curr, end, &maxFloatVal);
		if (peekChar(curr, end) != '>')
			lcRaiseError("Syntax Error", curr, 32);
		getChar(curr, end); // consume '>'
		
		currNode->addChild(new ASTNode(kTokenRangedLiteral, floatVal, maxFloatVal));
	}
	else if (hex)
	{
		unsigned int hexVal = 0;
		getChar(curr, end); // consume '#'
		curr = tsGetHex(curr, end, &hexVal);
		currNode->addChild(new ASTNode(kTokenIntLiteral, (int) hexVal));
	}
	else
	{
		TokenId token = getToken(curr, end);
		if (token == kTokenFalse)
		{
			currNode->addChild(new ASTNode(kTokenFalse));
		}
		else if (token == kTokenTrue)
		{
			currNode->addChild(new ASTNode(kTokenTrue));
		}
		else
			lcRaiseError("Syntax Error", curr, 32);
	}
	more(curr, end);
}

class JSONCallback : public Lab::Json::Callback
{
public:
    virtual void startArray()
    {
        nodestack.push_back(currNode);
        ASTNode* newNode = new ASTNode(kTokenDataArray);
        currNode->addChild(newNode);
        currNode = newNode;
    }
    
    virtual void endArray()
    {
        currNode = nodestack.back();
        nodestack.pop_back();
    }
    
    virtual void startObject()
    {
        nodestack.push_back(currNode);
        ASTNode* newNode = new ASTNode(kTokenDataObject);
        currNode->addChild(newNode);
        currNode = newNode;
    }
    
    virtual void nameValue(char const*const name, int len)
    {
        nodestack.push_back(currNode);
        char temp[256];
        strncpy(temp, name, len);
        temp[len] = '\0';
        ASTNode* newNode = new ASTNode(kTokenDataElement, temp);
        currNode->addChild(newNode);
        currNode = newNode;
    }
    
    virtual void endValue()
    {
        currNode = nodestack.back();
        nodestack.pop_back();
    }
    
    virtual void endObject()
    {
        currNode = nodestack.back();
        nodestack.pop_back();
    }

    virtual void nullVal()
    {
       currNode->addChild(new ASTNode(kTokenDataNullLiteral));
    }
                           
    virtual void boolVal(bool b)
    {
        currNode->addChild(new ASTNode(kTokenDataIntLiteral, b ? 1 : 0));
    }
                           
    virtual void strVal(char const*const str, int len)
    {
        char temp[256];
        strncpy(temp, str, len);
        temp[len] = '\0';
        currNode->addChild(new ASTNode(kTokenDataStringLiteral, temp));
    }

    virtual void floatVal(float f)
    {
        currNode->addChild(new ASTNode(kTokenDataFloatLiteral, f));
    }

    virtual void raiseError(char const*const curr, int len)
    {
        lcRaiseError("Problem parsing JSON", curr, len);
    }
    
    std::vector<ASTNode*> nodestack;
};

/* 
 globalVarDecls
 declarator EQUAL require LPAREN string RPAREN
 -> ^(TokenGlobalVariable declarator string)
 */

// assign-+
//  |     |
//  name  library

void parseGlobalVarDecls(CurrPtr& curr, EndPtr end, std::vector<std::pair<std::string, Json::Value*> >* jsonVars)
{
    char name[256];
    char require[256];
    getDeclarator(curr, end, name);
    
    CurrPtr t = curr;
    if (!getToken(kTokenEq, curr, end)) {
        lcRaiseError("Missing = after global declaration", curr, 32);
    }
    
    ASTNode* globVar = new ASTNode(kTokenGlobalVariable, name);
    currNode->addChild(globVar);
                                  
    if (peekChar('{', curr, end) || peekChar('[', curr, end)) {
        
        const char* jsonStart = curr;
        
        // Parse a JSON block.
        JSONCallback jsonCallback;
        ASTNode* pop = currNode;
        currNode = new ASTNode(kTokenStaticData);
        globVar->addChild(currNode);
        if (!Lab::Json::parseJsonValue(curr, end, &jsonCallback))
            lcRaiseError("Poorly formed JSON chunk", curr, 32);
        
        const char* jsonEnd = curr;
        
        Json::Reader reader;
        Json::Value* root = new Json::Value();
        reader.parse(jsonStart, jsonEnd, *root, false); // false = ignore comments
        
        jsonVars->push_back(std::pair<std::string, Json::Value*>(name, root));
        
        currNode = pop;
    }
	else if (getToken(kTokenRequire, curr, end)) {
        if (getChar('(', curr, end)) {
            curr = tsSkipCommentsAndWhitespace(curr, end);
            char const* value;
            uint32_t length;
            if (*curr == '"') {
                curr = tsGetString(curr, end, true, &value, &length);
                strncpy(require, value, length);
                require[length] = '\0';
            }
            curr = tsSkipCommentsAndWhitespace(curr, end);
            if (!getChar(')', curr, end)) {
                lcRaiseError("Poorly formed require", curr, 32);
            }
        }
        globVar->addChild(new ASTNode(kTokenRequire, require)); 
    }
    else {
        curr = t;
    }
}




/*
 landruBlock
 : lDeclareBlock | lParamBlock | lArrayBlock | lStateBlock ;
 */

void parseLandruBlock(CurrPtr& curr, EndPtr end)
{
	TokenId token = peekToken(curr, end);
	switch (token)
	{
		case kTokenDeclare:		parseDeclare(curr, end);                    break;
		case kTokenParam:		parseParamBlock(curr, end);                 break;
        case kTokenLocal:       parseLocals(curr, end);                     break;
		case kTokenState:		parseState(curr, end);                      break;
		default:				lcRaiseError("Unknown token", curr, 32);    break;
	}
}


/*
 A machine is a declarator followed by a number of blocks
 
 machine
 : MACHINE declarator COLON landruBlock+ SEMICOLON   
 -> ^(MACHINE declarator landruBlock+) ;
 */

void parseMachine(CurrPtr& curr, EndPtr end)
{
	TokenId token = getToken(curr, end);
	if (token != kTokenMachine) {
		lcRaiseError("Expected 'machine'", curr, 32);
		return;
	}
	
	char declarator[256];
	getDeclarator(curr, end, declarator);
	if (!getColon(curr, end))
		return;
	
	ASTNode* ast = new ASTNode(kTokenMachine, declarator);
	currNode->addChild(ast);
	ASTNode* pop = currNode;
	currNode = ast;
	
	++curr;
	while (more(curr, end))
	{
		if (*curr == kTerminalChar)
		{
			++curr;
			break;
		}
		parseLandruBlock(curr, end);
	}
	
	currNode = pop;
}

//-----------------------------------------------------------------------
//
// LANDRU PROGRAM
//
//-----------------------------------------------------------------------

/*
    zero or more(machine, globals)
 */	


extern "C"
void landruParseProgram(void* rootNode, 
                        std::vector<std::pair<std::string, Json::Value*> >* jsonVars,
                        char const* buff, size_t len)
{	
	currNode = (Landru::ASTNode*) rootNode;
	
	char const*const end = buff + len;
	char const* curr = buff;
	
	while (more(curr, end))
	{
		TokenId token = peekToken(curr, end);
		switch (token)
		{
			case kTokenMachine:
				parseMachine(curr, end);
				break;
				
			default:
                parseGlobalVarDecls(curr, end, jsonVars);
				break;
		}
	}
}


// ADDITIONAL UTILITY FUNCTIONS


extern "C"
void* landruCreateRootNode()
{
    Landru::ASTNode* node = new Landru::ASTNode(kTokenProgram, "Landru");
    return (void*) node;
}

extern "C"
void landruPrintAST(void* rootNode_)
{
    Landru::ASTNode* rootNode = (Landru::ASTNode*) rootNode_;
    if (rootNode)
        rootNode->print(0);
}

extern "C"
void landruPrintRawAST(void* rootNode_)
{
    Landru::ASTNode* rootNode = (Landru::ASTNode*) rootNode_;
    if (rootNode)
        rootNode->dump(0);
}

extern "C"
void landruToJson(void* rootNode_)
{
    Landru::ASTNode* rootNode = (Landru::ASTNode*) rootNode_;
    if (rootNode)
        rootNode->toJson();
}
