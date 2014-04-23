
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



namespace Landru
{
    class CompilationContext;
	class Exemplar;
    
	class Assembler : public AssemblerBase
	{
		std::map<std::string, int>	stateAddrMap;
		std::map<std::string, int>	stateOrdinalMap;
		int							maxStateOrd;
		
		std::map<std::string, int>	stringOrdinalMap;
		int							maxStringOrd;
		
		std::vector<unsigned int>	program;
		
		// key is name, value is type
		std::map<std::string, std::string>	varType;
		std::map<std::string, int>			varIndex;
		int									maxVarIndex;
        
		std::map<std::string, std::string>	sharedVarType;
		std::map<std::string, int>			sharedVarIndex;
		int									maxSharedVarIndex;
        
        struct LocalParameter
        {
            LocalParameter() { }
            LocalParameter(const LocalParameter& rhs) : name(rhs.name), type(rhs.type) {}
            LocalParameter(const char* name, const char* type) : name(name), type(type) {}
            std::string name;
            std::string type;
        };
        
        std::vector<LocalParameter> localParameters;

        CompilationContext* context;

	public:
        
        std::vector<std::shared_ptr<Exemplar>> exemplars;
        
		Assembler(CompilationContext* context);
		virtual ~Assembler();
        
        virtual void finalize();
		virtual void reset();
		virtual int stringIndex(const char* s);		
		int stateIndex(const char* s);
        virtual int instanceVarIndex(const char* varName);
		
		virtual void callFunction(const char* fnName);
        virtual void getRequire(int i);
		virtual void pushStringConstant(const char* str);
		virtual void pushVarIndex(const char* varName);
		virtual void pushGlobalVarIndex(const char* varName);
		virtual void pushSharedVarIndex(const char* varName);
		virtual void pushConstant(int i);
		virtual void pushFloatConstant(float v);
        virtual void createTempString();
		virtual void rangedRandom();
        virtual void pop();
		virtual void popStore();		
		virtual void pushIntOne();		
		virtual void pushIntZero();		
		virtual void stateEnd();		
		virtual void subStateEnd();
        virtual void paramsStart();
        virtual void paramsEnd();
		virtual void nop();		
        virtual void getGlobalVar();
		virtual void getSharedVar();
		virtual void getSelfVar(int);
        virtual void getLocalParam(int);
        virtual void forEach();
		virtual void onMessage();		
		virtual void onTick();		
		virtual int  gotoAddr();		
		virtual void gotoAddr(int addr);		
		virtual void gotoState(const char* stateName);
		virtual void launchMachine();
		virtual void ifEq();		
		virtual void ifLte0();		
		virtual void ifGte0();		
		virtual void ifLt0();		
		virtual void ifGt0();
		virtual void ifEq0();		
		virtual void ifNotEq0();
        
        virtual void opAdd();
        virtual void opSubtract();
        virtual void opMultiply();
        virtual void opDivide();
        virtual void opNegate();
        virtual void opModulus();
        
		virtual void _addState(const char* name);		
		virtual void _addVariable(const char* name, const char* type, bool shared);		
		virtual void _patchGoto(int patch);
		
        virtual void disassemble(const std::string& machineName, FILE* f);		
		virtual int programSize();		
		virtual std::unique_ptr<Exemplar> createExemplar();

        virtual bool isRequire(const char* name);
        virtual int  requireIndex(const char* name);
        virtual bool isGlobalVariable(const char* name);
        virtual bool isLocalParam(const char* name);
        virtual int  localParamIndex(const char* name);
        virtual void addLocalParam(const char* name, const char* type);
        virtual void callFactory();
        
        virtual void dotChain();

        virtual bool isSharedVariable(const char* name);
	};
	
	
	
} // Landru


#endif // cplusplus
