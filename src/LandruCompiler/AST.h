
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRU_AST_H
#define LANDRU_AST_H

#include "LandruCompiler/Tokens.h"
#include <string>
#include <vector>

namespace Landru
{
	class ASTNode;
	typedef std::vector<ASTNode*>::const_iterator ASTConstIter;

	class AssemblerBase;
	
	class ASTNode
	{
	public:
        
        enum UseMask { kNone, kInt, kFloat, kFloat2, kStr, kStr2 };
        
		ASTNode(TokenId token) : token(token), useMask(kNone)
		{
		}
		ASTNode(TokenId token, int i) : token(token), intVal(i), useMask(kInt)
		{
		}
		ASTNode(TokenId token, float f) : token(token), floatVal1(f), useMask(kFloat)
		{
		}
		ASTNode(TokenId token, float f1, float f2) : token(token), floatVal1(f1), floatVal2(f2), useMask(kFloat2)
		{
		}
		ASTNode(TokenId token, const char* str2) : token(token), str2(str2), useMask(kStr)
		{
		}
		ASTNode(TokenId token, const char* str1, const char* str2) : token(token), str1(str1), str2(str2), useMask(kStr2)
		{
		}
        
		~ASTNode();
		
		void addChild(ASTNode* ast)
		{
			children.push_back(ast);
		}

		//void compile(AssemblerBase& a);
        
        std::string toJson() const;
		
		void dump(int tabs=0) const;
		void print(int tabs=0) const;
        

		TokenId					token;
        UseMask                 useMask;
		std::string				str1;
		std::string				str2;
		float					floatVal1;
		float					floatVal2;
		int						intVal;
		std::vector<ASTNode*>	children;
        
        
	protected:
		static int inParamList;
		
		void printParameters(int tabs = 0) const;
		void printQualifier() const;
		void printChildParameters() const;
		void printStatements(int tabs) const;
		void printChildStatements(int tabs) const;
	};
	
} // Landru

#endif

