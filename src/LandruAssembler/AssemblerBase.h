
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
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "LandruActorVM/Property.h"

namespace Lab {
    class Bson;
}

namespace Landru {

    class ASTNode;

	class AssemblerBase {
	public:
        class Exception : public std::exception {
        public:
            Exception() {}
            Exception(const char* s) : s(s) {}
            Exception(const std::string& s) : s(s) {}
            Exception(const std::stringstream& str) : s(str.str()) {}
            virtual const char* what() const throw() { return s.c_str(); }
            std::string s;
        };
        #define AB_RAISE(text) do { stringstream s; s << text; throw Exception(s); } while (0)

		AssemblerBase() {}
        virtual ~AssemblerBase();

        virtual void startAssembling() = 0;
        virtual void finalizeAssembling() = 0;

        virtual void callFunction(const char* fnName) = 0;
        virtual void storeToVar(const char* varName) = 0;
    	virtual void initializeSharedVarIfNecessary(const char * varName) = 0;

		virtual void pushGlobalVar(const char* varName) = 0;
        virtual void pushInstanceVar(const char* varName) = 0;
        virtual void pushLocalVar(const char* varName) = 0;
		virtual void pushSharedVar(const char* varName) = 0;

        virtual void pushInstanceVarReference(const char* varName) = 0;
        virtual void pushGlobalVarReference(const char* varName) = 0;
        virtual void pushSharedVarReference(const char* varName) = 0;

        virtual void pushConstant(int) = 0;
        virtual void pushFloatConstant(float) = 0;
        virtual void pushRangedRandom(float r1, float r2) = 0;
        virtual void pushStringConstant(const char* str) = 0;

        // local parameters
        virtual void paramsStart() = 0;
        virtual void paramsEnd() = 0;

        virtual void beginForEach(const char* name, const char* type) = 0;
        virtual void endForEach() = 0;
        virtual void beginOn() = 0;
        virtual void beginOnStatements() = 0;
        virtual void endOnStatements() = 0;

        virtual void beginConditionalClause() = 0;
        virtual void beginContraConditionalClause() = 0;
        virtual void endConditionalClause() = 0;

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
        virtual void opGreaterThan() = 0;
        virtual void opLessThan() = 0;

		// variables
		virtual void addSharedVariable(const char* name, const char* type) = 0;
        virtual void addInstanceVariable(const char* name, const char* type) = 0;

        virtual void beginLocalVariableScope() = 0;
        virtual void addLocalVariable(const char* name, const char* type) = 0;
        virtual void endLocalVariableScope() = 0;

        // requires and globals
        virtual void addRequire(const char* name, const char* module) = 0;
		virtual void addGlobal(const char* name, const char* type) = 0;
#ifdef LANDRU_HAVE_BSON
        virtual void addGlobalBson(const char* name, std::shared_ptr<Lab::Bson>) = 0;
#endif
		virtual void addGlobalString(const char* name, const char* value) = 0;
		virtual void addGlobalInt(const char* name, int value) = 0;
		virtual void addGlobalFloat(const char* name, float value) = 0;

        virtual std::vector<std::string> requires() = 0;

        // parses the requires from the root node and returns the names
        std::vector<std::string> requires2(ASTNode*);

        // machines
        virtual void beginMachine(const char* name) = 0;
        virtual void endMachine() = 0;

		// states
		virtual void beginState(const char* name) = 0;
        virtual void endState() = 0;

        virtual void disassemble(const std::string& machineName, FILE* f) = 0;

        // dot chain means let the runtime know that the next function will be invoked on the stack top object
        virtual void dotChain() = 0;

        // assembler
        void assemble(ASTNode* root);

    protected:
        void assembleMachine(ASTNode* root);
        void assembleDeclarations(ASTNode* root);
        void assembleState(ASTNode* root);
        void assembleStatements(ASTNode* root);
        void assembleNode(ASTNode* root);

        bool isLocalVar(const char* name) const;

        std::vector<std::string> states;
        std::map<std::string, std::string> _requires;
        std::set<std::string> selfVarNames;
        std::map<std::string, std::string>	sharedVars;
		std::map<std::string, std::shared_ptr<Landru::Property>> globals;

        std::vector<std::vector<std::pair<std::string, std::string>>> scopedVariables; // stack of local variable scopes
	};

} // Landru

#endif
