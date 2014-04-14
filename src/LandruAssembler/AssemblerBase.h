
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRUASSEMBLER_ASSEMBLERBASE_H
#define LANDRUASSEMBLER_ASSEMBLERBASE_H

/*
 *  AssemblerBase.h
 *  Lab
 *
 *
 */

#include <stdio.h>
#include <string>
#include <vector>

namespace Landru 
{
	class ASTNode;

	class AssemblerBase
	{
	public:
		AssemblerBase() { }
		virtual ~AssemblerBase() { }
		
		virtual void callFunction(const char* fnName) = 0;
		virtual void pushStringConstant(const char* str) = 0;
		virtual void pushVarIndex(const char* varName) = 0;
		virtual void pushGlobalVarIndex(const char* varName) = 0;
		virtual void pushSharedVarIndex(const char* varName) = 0;
		virtual void pushConstant(int) = 0;
		virtual void pushFloatConstant(float) = 0;
		virtual void rangedRandom() = 0;
        virtual void createTempString() = 0;
		virtual void popStore() = 0;
		virtual void pushIntOne() = 0;
		virtual void pushIntZero() = 0;
		virtual void stateEnd() = 0;
		virtual void subStateEnd() = 0;
        virtual void paramsStart() = 0;
        virtual void paramsEnd() = 0;
		virtual void nop() = 0;
		virtual void getGlobalVar() = 0;
		virtual void getSharedVar() = 0;
		virtual void getSelfVar() = 0;
        virtual void getLocalParam(int) = 0; // int is an index into the surrounding stack frame
        virtual void forEach() = 0;
		virtual void onMessage() = 0;
		virtual void onTick() = 0;
        virtual void getRequire(int i) = 0;
		virtual int  gotoAddr() = 0;	// returns address to patch
		virtual void gotoAddr(int) = 0; // use when destination address is already known
		virtual void gotoState(const char* stateName) = 0;
		virtual void launchMachine() = 0;
		virtual void ifEq() = 0;
		virtual void ifLte0() = 0;
		virtual void ifGte0() = 0;
		virtual void ifLt0() = 0;
		virtual void ifGt0() = 0;
		virtual void ifEq0() = 0;
		virtual void ifNotEq0() = 0;
        
        virtual void opAdd() = 0;
        virtual void opSubtract() = 0;
        virtual void opMultiply() = 0;
        virtual void opDivide() = 0;
        virtual void opNegate() = 0;
        virtual void opModulus() = 0;
		
		// variables
		virtual void _addVariable(const char* name, const char* type, bool shared) = 0;
        virtual bool isGlobalVariable(const char* name) = 0;
        virtual bool isSharedVariable(const char* name) = 0;

        // requires
        virtual bool isRequire(const char* name) = 0;
        virtual int  requireIndex(const char* name) = 0;

		// states
		virtual void _addState(const char* name) = 0;
		
		// housekeeping
        virtual void finalize() = 0;
		virtual void reset() = 0;
		virtual void _patchGoto(int patch) = 0;
		virtual int  stringIndex(const char*) = 0;
		virtual void disassemble(const std::string& machineName, FILE* f) = 0;
		virtual int  programSize() = 0;

        // local parameters
        virtual void pushLocalParameters() = 0;
        virtual void popLocalParameters() = 0;
        virtual bool isLocalParam(const char* name) = 0;
        virtual int  localParamIndex(const char* name) = 0;
        virtual void addLocalParam(const char* name, const char* type) = 0;
        virtual void callFactory() = 0;
        
        virtual void dotChain() = 0;
        
        // assembler
        void assemble(ASTNode* root);
        
    protected:
        void assembleMachine(ASTNode* root);
        void assembleDeclarations(ASTNode* root);
        void assembleState(ASTNode* root);
        void assembleStatements(ASTNode* root);
        void assembleNode(ASTNode* root);

        std::vector<std::string> states;
	};

} // Landru

#endif
