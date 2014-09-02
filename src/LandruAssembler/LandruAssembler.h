
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
#include "LabJson/bson.h"

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
		
		// key is name, value is type
		std::map<std::string, std::string>	varType;
		std::map<std::string, int>			varIndex;
		int maxVarIndex;
        
		std::map<std::string, std::string>	sharedVarType;
		std::map<std::string, int>			sharedVarIndex;
		int maxSharedVarIndex;
        
        std::vector<int> localVariableState;
        
        struct LocalParameter {
            LocalParameter() {}
            LocalParameter(const LocalParameter& rhs) : name(rhs.name), type(rhs.type) {}
            LocalParameter(const char* name, const char* type) : name(name), type(type) {}
            std::string name;
            std::string type;
        };
        
        std::vector<LocalParameter> localParameters;
        std::vector<int> clauseStack;

        CompilationContext* context;

        void addLocalParam(const char* name, const char* type);
        void subStateEnd();
        void popStore();
        void popLocal();
        void nop();
        int  gotoAddr();
        void gotoAddr(int addr);
        void _patchGoto(int patch);
        void localParamPop();

	public:
        std::vector<std::shared_ptr<Exemplar>> exemplars;
        
		Assembler(CompilationContext* context);
		virtual ~Assembler();
        
        int programSize();
        int stateIndex(const char* s);
        std::unique_ptr<Exemplar> createExemplar();
        virtual void callFactory();
        
        virtual void startAssembling() override;
        virtual void finalizeAssembling() override;
		virtual int stringIndex(const char* s) override;
        virtual int instanceVarIndex(const char* varName) override;
		
		virtual void callFunction(const char* fnName) override;
        virtual void getRequire(int i) override;
		virtual void pushStringConstant(const char* str) override;
        virtual void storeToVar(const char* varName) override;
		virtual void pushGlobalVarIndex(const char* varName) override;
		virtual void pushSharedVarIndex(const char* varName) override;
		virtual void pushConstant(int i) override;
		virtual void pushFloatConstant(float v) override;
        virtual void createTempString() override;
		virtual void rangedRandom() override;
		virtual void pushIntOne() override;
        virtual void pushIntZero() override;
		virtual void stateEnd() override;
        virtual void paramsStart() override;
        virtual void paramsEnd() override;
        virtual void getGlobalVar() override;
		virtual void getSharedVar() override;
		virtual void getSelfVar(int) override;
        virtual void getLocalParam(int) override;
        virtual void beginForEach(const char* name, const char* type) override;
        virtual void endForEach() override;
        virtual void beginOn() override;
        virtual void endOn() override;
        
        virtual void beginConditionalClause() override;
        virtual void beginContraConditionalClause() override;
        virtual void endConditionalClause() override;
        
		virtual void gotoState(const char* stateName) override;
		virtual void launchMachine() override;
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
        
		virtual void _addState(const char* name) override;
		virtual void _addVariable(const char* name, const char* type, bool shared) override;
		
        virtual void disassemble(const std::string& machineName, FILE* f) override;

        virtual bool isRequire(const char* name) override;
        virtual int  requireIndex(const char* name) override;
        virtual bool isGlobalVariable(const char* name) override;
        virtual bool isLocalParam(const char* name) override;
        virtual int  localParamIndex(const char* name) override;
        
        virtual void beginLocalVariableScope() override;
        virtual void addLocalVariable(const char* name, const char* type) override;
        virtual void endLocalVariableScope() override;
        
        virtual void dotChain() override;

        virtual bool isSharedVariable(const char* name) override;
	};
	
	
	
} // Landru


#endif // cplusplus
