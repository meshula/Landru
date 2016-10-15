
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruCompiler/AST.h"
#include "LandruCompiler/lcRaiseError.h"

#include <string>
#include <map>
#include <set>

namespace Landru
{
	int ASTNode::inParamList = 0;
	
	ASTNode::~ASTNode()
	{
		for (ASTConstIter i = children.begin(); i != children.end(); ++i)
			delete *i;
	}
	
	
	void ASTNode::printParameters(int tabs) const
	{
		++inParamList;
		switch (token)
		{
			case kTokenParameters:
				if (children.begin() != children.end()) {
					ASTConstIter i = children.begin();
					for ( ; i != children.end(); ++i) {
						(*i)->print(tabs);
					}
				}
				break;
			default:
				lcRaiseError("AST Error: This node is supposed to be a parameter node", 0, 0);
				break;
		}
		--inParamList;
	}
	
	void ASTNode::printQualifier() const
	{
		switch (token)
		{
			case kTokenEq:
			case kTokenLte0:
			case kTokenGte0:
			case kTokenLt0:
			case kTokenGt0:
			case kTokenEq0:
			case kTokenNotEq0:
				printf("%s", tokenName(token));
				break;
				
			case kTokenFunction:
				printf("%s", str2.c_str());
				break;
				
			default:
				lcRaiseError("AST Error: Unknown qualifier\n", 0, 0);
				break;
		}
	}
	
	void ASTNode::printChildParameters() const
	{
		ASTConstIter i = children.begin();
		if (i != children.end())
		{
			(*i)->printParameters();
			++i;
			if (i != children.end())
				lcRaiseError("AST Error: Found more than just parameters under this node", 0, 0);
		}
		else
		{
			lcRaiseError("AST Error: parameters missing", 0, 0);
		}
	}
	
	void ASTNode::printChildStatements(int tabs) const
	{
		ASTConstIter i = children.begin();
		if (i != children.end())
		{
			(*i)->printStatements(tabs);
			++i;
			if (i != children.end())
				lcRaiseError("AST Error: Found more than just statements under this node\n", 0, 0);
		}
		else
		{
			lcRaiseError("AST Error: statements missing\n", 0, 0);
		}
	}
	
	void ASTNode::printStatements(int tabs) const
	{
		if (token != kTokenStatements)
		{
			lcRaiseError("AST Error: should be statements token\n", 0, 0);
		}
		for (ASTConstIter i = children.begin(); i != children.end(); ++i)
		{
			(*i)->print(tabs);
		}
	}
	
	void ASTNode::print(int tabs) const
	{
		static char* tabChars = (char*) "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
		static int maxTabs = (int) strlen(tabChars);
		if (tabs > maxTabs)
			tabs = maxTabs;
		const char* tabStr = tabChars + maxTabs - tabs;

		ASTConstIter i = children.begin();
		switch (token)
		{
			case kTokenDeclare:
				printf("%s%s:\n", 
					   tabStr, 
					   tokenName(token));
				for ( ; i != children.end(); ++i)
					(*i)->print(tabs+1);
				printf("%s;\n\n", tabStr);
				break;
                                
            case kTokenDataObject:
                if (children.size() == 0)
                    printf("%s{}\n", tabStr);
                else {
                    printf("%s{", tabStr);
                    for ( ; i != children.end(); ++i)
                        (*i)->print(tabs+1);
                    printf("%s}\n", tabStr);
                }
                break;
                
            case kTokenDataArray:
                if (children.size() == 0)
                    printf("%s[]\n", tabStr);
                else {
                    printf("%s[", tabStr);
                    for ( ; i != children.end(); ++i)
                        (*i)->print(tabs+1);
                    printf("%s]\n", tabStr);
                }
                break;
                
            case kTokenStaticData:
                for ( ; i != children.end(); ++i)
                    (*i)->print(tabs+1);
                break;
                
            case kTokenRequire:
                printf("%srequire(\"%s\")\n", tabStr, str2.c_str());
                break;
            
            case kTokenDataElement:
                printf("%s\"%s\" : ", tabStr, str2.c_str());
                break;
				
			case kTokenStringLiteral:
				printf("%s\"%s\" ", tabStr, str2.c_str());
				break;
				
			case kTokenIntLiteral:
				printf("%s%d ", tabStr, intVal);
				break;
				
			case kTokenFloatLiteral:
				printf("%s%f ", tabStr, floatVal1);
				break;
				
			case kTokenRangedLiteral:
				printf("%s<%f, %f> ", tabStr, floatVal1, floatVal2);
				break;
				
			case kTokenGetVariable:
				printf("%s%s ", tabStr, str2.c_str());
				break;
				
			case kTokenAssignment:
				printf("%s%s = ", tabStr, str2.c_str());
				/// @TODO, assignment should have a parameter list under it
				(*i)->print(tabs+1);
				++i;
				break;
				
			case kTokenSharedVariable:
				printf("%sshared %s %s\n", 
					   tabStr, 
					   str1.c_str(), 
					   str2.c_str());
				break;
				
			case kTokenLocalVariable:
				printf("%s%s %s\n", 
					   tabStr, 
					   str1.c_str(), 
					   str2.c_str());
				break;
				
			case kTokenGlobalVariable:
				printf("\n%s%s = ", 
					   tabStr, 
					   str2.c_str());
				for ( ; i != children.end(); ++i)
					(*i)->print(tabs);
				break;
				
			case kTokenIf:
			case kTokenOn:
				printf("%s%s ", tabStr, tokenName(token));
				if (i != children.end())
				{
					(*i)->printQualifier();
                    
                    if ((*i)->children.size() > 0) {
                        printf("(");
                        (*i)->printChildParameters();
                        printf(")");
                    }
                    printf(":\n");
					++i;
					(*i)->printStatements(tabs+1);
					++i;
					printf("%s;\n", tabStr);
					if (i != children.end())
					{
						// else
						printf("%s%s:\n", tabStr, tokenName((*i)->token));
						(*i)->printChildStatements(tabs+1);
						++i;
						printf("%s;\n", tabStr);
					}
				}
				break;
								
			case kTokenFunction:
			case kTokenLaunch:
				if (token == kTokenFunction)
					printf("%s%s(", tabStr, str2.c_str());
				else
					printf("%s%s(", tabStr, tokenName(token));
				(*i)->printParameters();
				printf(")%s", inParamList ? " " : "\n");
				++i;
				break;
				
            case kTokenDotChain:
			case kTokenTrue:
			case kTokenFalse:
				printf("%s ", tokenName(token));
				break;
				
			case kTokenMachine:
				printf("\n%s%s %s:\n", tabStr, tokenName(token), str2.c_str());
				for ( ; i != children.end(); ++i)
					(*i)->print(tabs+1);
				printf("%s;\n\n", tabStr);
				break;
				
			case kTokenState:
				printf("%s%s %s:\n", tabStr, tokenName(token), str2.c_str());
				(*i)->printStatements(tabs+1);
				++i;
				printf("%s;\n\n", tabStr);
				break;
				
			case kTokenProgram:
				printf("//%s%s %s:\n", tabStr, tokenName(token), str2.c_str());
				for ( ; i != children.end(); ++i)
					(*i)->print(tabs);
				break;
				
			case kTokenGoto:
				printf("%s%s %s\n", tabStr, tokenName(token), str2.c_str());
				break;
                
			default:
                {
                    const char* tn = tokenName(token);
                    lcRaiseError("AST Error: Unhandled token\n", tn, (int) strlen(tn));
                }
                break;
		}
		
		if (i != children.end())
		{
			lcRaiseError("AST Error: children dangling\n", 0, 0);
		}
	}
	
	
	void ASTNode::dump(int tabs) const
	{
		static char* tabChars = (char*) "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
		static int maxTabs = static_cast<int>(strlen(tabChars));
		if (tabs > maxTabs)
			tabs = maxTabs;
		const char* tabStr = tabChars + maxTabs - tabs;
		
		printf("%s%s: %s\n", tabStr, tokenName(token), str2.c_str());
		for (ASTConstIter i = children.begin(); i != children.end(); ++i) {
			(*i)->dump(tabs+1);
		}
	}
    
    std::string ASTNode::toJson() const
    {
        printf("{ \"%s\" : \"%s\"", tokenName(token), str2.c_str());
        if (children.size() > 0) {
            printf(", c : [\n");
            for (ASTConstIter i = children.begin(); i != children.end(); ++i) {
                (*i)->toJson();
                printf(",");
            }
            printf("]\n");
        }
        printf("}\n");
        
        return "";
    }

	

	

} // Landru

