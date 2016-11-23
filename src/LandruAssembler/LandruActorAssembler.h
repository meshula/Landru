
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

namespace Lab {
    class Bson;
}

namespace Landru {

    class Library;
    class MachineDefinition;

    class ActorAssembler : public AssemblerBase {
        class Context;
        std::unique_ptr<Context> _context;
    public:
        ActorAssembler();
        virtual ~ActorAssembler();

        Library& library() const;

        const std::map<std::string, std::shared_ptr<Lab::Bson>>& assembledGlobalVariables() const;
        const std::map<std::string, std::shared_ptr<MachineDefinition>>& assembledMachineDefinitions() const;

        virtual void startAssembling() override {}
        virtual void finalizeAssembling() override {}

        virtual void callFunction(const char* fnName) override;
        virtual void storeToVar(const char* varName) override;
     	virtual void initializeSharedVarIfNecessary(const char * varName) override;

        virtual void pushConstant(int) override;
        virtual void pushFloatConstant(float) override;
        virtual void pushGlobalVar(const char* varName) override;
        virtual void pushInstanceVar(const char* varName) override;
        virtual void pushLocalVar(const char* varName) override;
        virtual void pushRangedRandom(float r1, float r2) override;
        virtual void pushRequire(const char* name) override;
        virtual void pushSharedVar(const char* varName) override;
        virtual void pushStringConstant(const char* str) override;

        // local parameters
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
        virtual void opGreaterThan() override;
        virtual void opLessThan() override;

        // variables
        virtual void addSharedVariable(const char* name, const char* type) override;
        virtual void addInstanceVariable(const char* name, const char* type) override;

        virtual void beginLocalVariableScope() override;
        virtual void addLocalVariable(const char* name, const char* type) override;
        virtual void endLocalVariableScope() override;

        // requires
        virtual void addRequire(const char* name, const char* module) override;
        virtual void addGlobalBson(const char* name, std::shared_ptr<Lab::Bson>) override;

        // machines
        virtual void beginMachine(const char* name) override;
        virtual void endMachine() override;

        // states
        virtual void beginState(const char* name) override;
        virtual void endState() override;

        virtual void disassemble(const std::string& machineName, FILE* f) override {}

        // dot chain means let the runtime know that the next function will be invoked on the stack top object
        virtual void dotChain() override {}
    };



} // Landru


#endif // cplusplus
