
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

/*
 *  MachineAssembler.h
 *  Lab
 *
 */

#pragma once

#ifndef EXTERNC
# ifdef __cplusplus
#  define EXTERNC extern "C"
# else
#  define EXTERNC
# endif
#endif

#include <map>
#include <string>
#include <vector>

namespace Json { class Value; }

EXTERNC void landruAssembleProgram(void* rootNode);
EXTERNC void landruAssembleProgramToRuntime(void* rootNode, void* runtime, std::vector<std::pair<std::string, Json::Value*> >*);

#ifdef __cplusplus

#include "LandruAssembler/AssemblerBase.h"
#include <map>
#include <vector>
#include <string>



namespace Landru {
    class CompilationContext;
	class Exemplar;

	class Assembler : public AssemblerBase {
		std::map<std::string, int>	stateAddrMap;
		std::map<std::string, int>	stateOrdinalMap;
		int maxStateOrd;

		std::map<std::string, int>	stringOrdinalMap;
		int maxStringOrd;

		std::vector<unsigned int>	program;

        std::string machineName;

		// key is name, value is type
		std::map<std::string, std::string>	varType;
		std::map<std::string, int>			varIndex;
		int maxVarIndex;

		std::map<std::string, std::string>	sharedVarType;
		std::map<std::string, int>			sharedVarIndex;
		int maxSharedVarIndex;

        std::vector<int> localVariableState;

        struct LocalVariable {
            LocalVariable() {}
            LocalVariable(const LocalVariable& rhs) : name(rhs.name), type(rhs.type) {}
            LocalVariable(const char* name, const char* type) : name(name), type(type) {}
            std::string name;
            std::string type;
        };

        std::vector<LocalVariable> localVariables;
        std::vector<int> clauseStack;

        CompilationContext* context;

        void subStateEnd();
        void popStore();
        void popLocal();
        void nop();
        int  gotoAddr();
        void gotoAddr(int addr);
        void _patchGoto(int patch);

        int programSize();
        int stateIndex(const char* s);
        std::unique_ptr<Exemplar> createExemplar();
        void callFactory();
        int stringIndex(const char* s);

        void pushIntOne();
        void pushIntZero();
        void createTempString();
        void getGlobalVar();
        int  requireIndex(const char* name);
        void pushSharedVarIndex(const char* varName);
        int instanceVarIndex(const char* varName);
        int localVariableIndex(const char* name);

	public:
        std::vector<std::shared_ptr<Exemplar>> exemplars;

		Assembler(CompilationContext* context);
		virtual ~Assembler();

        virtual void startAssembling() override;
        virtual void finalizeAssembling() override;

        virtual void callFunction(const char* fnName) override;
        virtual void storeToVar(const char* varName) override;
    	virtual void initializeSharedVarIfNecessary(const char * varName) override;

        virtual void pushConstant(int i) override;
        virtual void pushFloatConstant(float v) override;
        virtual void pushGlobalVar(const char* varName) override;
		virtual void pushGlobalBsonVar(const char* varName) override;
		virtual void pushInstanceVar(const char* varName) override;
        virtual void pushLocalVar(const char* varName) override;
        virtual void pushRangedRandom(float r1, float r2) override;
#ifdef HAVE_VMCONTEXT_REQUIRES
        virtual void pushRequire(const char* name) override;
#endif
        virtual void pushSharedVar(const char* varName) override;
        virtual void pushStringConstant(const char* str) override;

        virtual void paramsStart() override;
        virtual void paramsEnd() override;
        virtual void beginForEach(const char* name, const char* type) override;
        virtual void endForEach() override;

        virtual void beginOn() override;
        virtual void beginOnStatements() override;
        virtual void endOnStatements() override;

        virtual void beginConditionalClause() override;
        virtual void beginContraConditionalClause() override;
        virtual void endConditionalClause() override;

        // machines
        virtual void beginMachine(const char* name) override;
        virtual void endMachine() override;
        virtual void launchMachine() override;

		virtual void gotoState(const char* stateName) override;
		virtual void ifEq() override;
		virtual void ifLte0() override;
		virtual void ifGte0() override;
		virtual void ifLt0() override;
		virtual void ifGt0() override;
		virtual void ifEq0() override;
		virtual void ifNotEq0() override;

        virtual void opAdd() override;
        virtual void opSubtract() override;
        virtual void opMultiply() override;
        virtual void opDivide() override;
        virtual void opNegate() override;
        virtual void opModulus() override;
        virtual void opGreaterThan() override;
        virtual void opLessThan() override;

		virtual void beginState(const char* name) override;
        virtual void endState() override;
		virtual void addSharedVariable(const char* name, const char* type) override;
        virtual void addInstanceVariable(const char* name, const char* type) override;

        virtual void disassemble(const std::string& machineName, FILE* f) override;

        virtual void addGlobalBson(const char* name, std::shared_ptr<Lab::Bson> b) override;
		virtual void addGlobalString(const char* name, const char* value) override;
		virtual void addGlobalInt(const char* name, int value) override;
		virtual void addGlobalFloat(const char* name, float value) override;


        virtual void addRequire(const char* name, const char* module) override;
		virtual std::vector<std::string> requires() override { return std::vector<std::string>();  }

        virtual void beginLocalVariableScope() override;
        virtual void addLocalVariable(const char* name, const char* type) override;
        virtual void endLocalVariableScope() override;

        virtual void dotChain() override;
	};



} // Landru


#endif // cplusplus
