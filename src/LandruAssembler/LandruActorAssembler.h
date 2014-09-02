
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

/*
 *  LandruActorAssembler.h
 *  Lab
 *
 */

#pragma once

#ifdef __cplusplus

#include "LandruAssembler/AssemblerBase.h"

namespace Landru {
    
    class ActorAssembler : public AssemblerBase {
    public:
        virtual void callFunction(const char* fnName) {}
        virtual void pushStringConstant(const char* str) {}
        virtual void storeToVar(const char* name) {}
        virtual void pushGlobalVarIndex(const char* varName) {}
        virtual void pushSharedVarIndex(const char* varName) {}
        virtual void pushConstant(int) {}
        virtual void pushFloatConstant(float) {}
        virtual void rangedRandom() {}
        virtual void createTempString() {}
        virtual void pushIntOne() {}
        virtual void pushIntZero() {}
        virtual void stateEnd() {}
        virtual void paramsStart() {}
        virtual void paramsEnd() {}
        virtual void getGlobalVar() {}
        virtual void getSharedVar() {}
        virtual void getSelfVar(int) {}
        virtual void getLocalParam(int) {} // int is an index into the surrounding stack frame
        virtual void beginForEach(const char* name, const char* type) override {}
        virtual void endForEach() override {}
        virtual void getRequire(int i) {}
        virtual void gotoState(const char* stateName) {}
        virtual void launchMachine() {}
        virtual void ifEq() {}
        virtual void ifLte0() {}
        virtual void ifGte0() {}
        virtual void ifLt0() {}
        virtual void ifGt0() {}
        virtual void ifEq0() {}
        virtual void ifNotEq0() {}
        
        virtual void opAdd() {}
        virtual void opSubtract() {}
        virtual void opMultiply() {}
        virtual void opDivide() {}
        virtual void opNegate() {}
        virtual void opModulus() {}
        
        // variables
        virtual void _addVariable(const char* name, const char* type, bool shared) {}
        virtual bool isGlobalVariable(const char* name) { return false; }
        virtual bool isSharedVariable(const char* name) { return false; }
        virtual int instanceVarIndex(const char* varName) { return 0; }
        
        // requires
        virtual bool isRequire(const char* name) { return false; }
        virtual int  requireIndex(const char* name) { return 0; }
        
        // states
        virtual void _addState(const char* name) {}
        
        // housekeeping
        virtual void finalizeAssembling() {}
        virtual void startAssembling() {}
        
        virtual int  stringIndex(const char*) { return 0; }
        virtual void disassemble(const std::string& machineName, FILE* f) {}
        
        // local parameters
        virtual bool isLocalParam(const char* name) { return false; }
        virtual int  localParamIndex(const char* name) { return 0; }
        virtual void addLocalVariable(const char* name, const char* type) override {}
        
        virtual void dotChain() {}
    };
    
    
    
} // Landru


#endif // cplusplus
