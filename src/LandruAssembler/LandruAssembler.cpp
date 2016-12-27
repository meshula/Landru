
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT


#include "LandruAssembler.h"
#include "LandruVM/Exemplar.h"
#include "LandruVM/Instructions.h"
#include "LandruCompiler/lcRaiseError.h"
#include "LandruStd/JsonVarObj.h"
#include "LabJson/bson.h"

#include "LandruCompiler/AST.h"
#include "LabText/TextScanner.hpp"
#include "LandruVM/Engine.h"
#include "LandruVM/VarObj.h"
#include "LandruVM/MachineCache.h"

#include <string.h>

//#define ASSEMBLER_TRACE(a) printf("Assembler: %s\n", #a)
#define ASSEMBLER_TRACE(a)

using namespace std;

namespace Landru {



    class CompilationContext
	{
    public:

        ~CompilationContext() {
        }

        int requiresIndex(const char* name) {
            string n(name);
            int i = 0;
            for (auto s : requiresList) {
                if (s.first == n || s.second == n)
                    return i;
                ++i;
            }
            return -1;
        }



        void compile(Landru::ASTNode* programNode)
        {
            Landru::Assembler a(this);
            a.assemble(programNode);
            for (auto ex: a.exemplars) {
                exemplars.push_back(ex);
            }
        }

        void copyToRuntime(Landru::Engine* e, std::vector<std::pair<std::string, Json::Value*> >* jsonVars)
        {
            for (int i = 0; i < requiresList.size(); ++i) {
                e->AddRequire(requiresList[i].first.c_str(), i);
            }

            for (auto i = jsonVars->begin(); i != jsonVars->end(); ++i) {
                Landru::JsonVarObj* jvo = new Landru::JsonVarObj(i->first.c_str());
                shared_ptr<Landru::VarObj>vp(jvo);
                jvo->setJson(i->second);
                e->AddGlobal(i->first.c_str(), vp);
            }

            Landru::MachineCache* m = e->machineCache();
            for (auto i = exemplars.begin(); i != exemplars.end(); ++i) {
                m->add(e->factory(), *i);
            }
        }

        vector<pair<string, string>> requiresList;
        vector<shared_ptr<Landru::Exemplar>> exemplars;

    }; // CompilationContext




    Assembler::Assembler(CompilationContext* cc)
    : context(cc) {
        program.reserve(1000);
        startAssembling();
    }

    Assembler::~Assembler() {}

    int Assembler::requireIndex(const char* name) {
        string s(name);
        for (int i = 0; i < context->requiresList.size(); ++i) {
            if (s == context->requiresList[i].first || s == context->requiresList[i].second)
                return i;
        }
        return -1;
    }

    int Assembler::localVariableIndex(const char* name) {
        if (localVariables.empty())
            return -1;

        // parameters were parsed in order into the vector;
        // they are indexed backwards from the local stack frame, so if the parameters were a, b, c
        // the runtime indices are a:2 b:1 c:0
        // search from most recently pushed because of scoping, eg in this case
        // for x in range(): for y in range(): local real x ...... ; ;
        // the innermost x should be found within the y loop, and the outermost in the x loop
        int m = (int) localVariables.size() - 1;
        for (int i = m; i >= 0; --i)
            if (!strcmp(name, localVariables[i].name.c_str()))
                return int(m - i);
        return -1;
    }

    void Assembler::beginLocalVariableScope() {
        localVariableState.push_back(0);
    }

    void Assembler::addLocalVariable(const char *name,  const char* type) {
        localVariables.push_back(LocalVariable(name, type));
        pushConstant(stringIndex(name));
        pushConstant(stringIndex(type));
        callFactory();
        localVariableState[localVariableState.size()-1] += 1;
    }

    void Assembler::endLocalVariableScope() {
        int localsCount = localVariableState[localVariableState.size()-1];
        localVariableState.pop_back();

        for (int i = 0; i < localsCount; ++i) {
            popLocal();
            localVariables.pop_back();
        }
    }

    void Assembler::popLocal() {
        program.push_back(Instructions::iPopLocal);
    }

    void Assembler::callFactory() {
        program.push_back(Instructions::iFactory);
    }


    void Assembler::startAssembling()
    {
    }

    void Assembler::finalizeAssembling()
    {
    }

    void Assembler::addRequire(const char* name, const char* module)
    {
        context->requiresList.push_back(pair<string,string>(name, module));
    }

    int Assembler::stringIndex(const char* s)
    {
        std::map<std::string, int>::const_iterator i = stringOrdinalMap.find(s);
        if (i != stringOrdinalMap.end()) {
            return i->second;
        }
        else {
            stringOrdinalMap[s] = maxStringOrd;
            int ret = maxStringOrd;
            ++maxStringOrd;
            return ret;
        }
    }

    int Assembler::stateIndex(const char* s)
    {
        std::map<std::string, int>::const_iterator i = stateOrdinalMap.find(s);
        if (i != stateOrdinalMap.end()) {
            return i->second;
        }
        else {
            stateOrdinalMap[s] = maxStateOrd;
            int ret = maxStateOrd;
            ++maxStateOrd;
            return ret;
        }
    }

    void Assembler::callFunction(const char* fnName)
    {
        char const*const dot = strchr(fnName, '.');
        if (dot) {
            vector<string> parts = TextScanner::Split(string(fnName), string("."));
            int index = 0;

            // a dotted function will either be on a local parameter, a require, or an a local variable, or on a class shared variable
            int i = localVariableIndex(parts[index].c_str());
            if (i >= 0) {
                // local parameter, push the local parameter index
                int e = i << 16;
                program.push_back(e | Instructions::iGetLocalVarObj);
            }
            else {
                i = context->requiresIndex(parts[index].c_str());
                if (i >= 0) {
                    // it's a require, push the require index
                    int e = i << 16;
                    program.push_back(e | Instructions::iGetRequire);
                }
                else {
                    // is it an instance or shared variable?
                    std::map<std::string, int>::const_iterator i = varIndex.find(parts[index].c_str());
                    if (i == varIndex.end()) {
                        RaiseError(0, "Unknown Variable", parts[index].c_str());
                    }
                    else {
                        if (sharedVarType.find(parts[index]) != sharedVarType.end()) {
                            /// @TODO make shared variables a separate chunk
                            // the convention is shared variable indices start at maxVarIndices
                            int index = (i->second + maxVarIndex) << 16;
                            program.push_back(index | Instructions::iGetSharedVar);
                        }
                        else {
                            program.push_back((i->second << 16) | Instructions::iGetSelfVar);
                        }
                    }
                }
            }

            // second part will either be local or class shared on the var on stack top
            ++index;
            i = stringIndex(parts[index].c_str()) << 16;
            if (parts.size() == 2) {
                // push the local string index because at compile time, the function index on the target object is not known
                program.push_back(i | Instructions::iCallFunction);
            }
            else {
                program.push_back(i | Instructions::iGetVarFromVar);
                ++index;
                i = stringIndex(parts[index].c_str()) << 16;
                if (parts.size() == 3) {
                    // push the local string index because at compile time, the function index on the target object is not known
                    program.push_back(i | Instructions::iCallFunction);
                }
                else {
                    RaiseError(0, "TODO Finish rewriting this as a recurrence", 0);
                    int i = stringIndex(parts[index].c_str()) << 16;
                    program.push_back(i | Instructions::iGetVarFromVar);
                    ++index;
                }
            }
        }
        else {
            int index = stringIndex(fnName) << 16;
            program.push_back(index | Instructions::iCallFunction);
        }
    }

    void Assembler::dotChain()
    {
        program.push_back(Instructions::iDotChain); // the instruction just pops the stack
    }

    void Assembler::pushStringConstant(const char* str)
    {
        pushConstant(stringIndex(str));
        createTempString();  // creates a temp StringVarObj, valid only until the semicolon is hit, all temps are reaped at the end of this run loop
    }

    void Assembler::createTempString()
    {
        program.push_back(Instructions::iCreateTempString);
    }

    int Assembler::instanceVarIndex(const char* varName) {
        std::map<std::string, int>::const_iterator i = varIndex.find(varName);
        if (i != varIndex.end())
            return i->second;
        return -1;
    }

    void Assembler::storeToVar(const char *varName)
	{
        std::map<std::string, int>::const_iterator i = varIndex.find(varName);
        if (i == varIndex.end()) {
            i = sharedVarIndex.find(varName);
            if (i == sharedVarIndex.end()) {
                RaiseError(0, "Unknown Variable", varName);
            }
            else {
                // the convention is shared variable indices start at maxVarIndices
                pushConstant(i->second + maxVarIndex);
            }
        }
        else {
            pushConstant(i->second);
        }
        popStore();
    }

	void Assembler::initializeSharedVarIfNecessary(const char * varName)
	{
        auto i = sharedVarIndex.find(varName);
        if (i == sharedVarIndex.end()) {
            RaiseError(0, "Unknown shared variable", varName);
        }
        else {
            // the convention is shared variable indices start at maxVarIndices
            // @TODO should only store if never stored
            pushConstant(i->second + maxVarIndex);
            popStore();
        }
	}

    void Assembler::pushSharedVarIndex(const char* varName)
    {
        std::map<std::string, int>::const_iterator i = sharedVarIndex.find(varName);
        if (i == varIndex.end()) {
            RaiseError(0, "Couldn't find shared variable", varName);
        }
        else {
            pushConstant(i->second);
        }
    }

	void Assembler::pushGlobalVar(const char * varName)
	{
		pushConstant(stringIndex(varName));
		getGlobalVar();
	}

    void Assembler::pushInstanceVarReference(const char* varName)
	{
		RaiseError(0, "pushGlobalVar not implemented", varName);
	}
    void Assembler::pushGlobalVarReference(const char* varName)
	{
		RaiseError(0, "pushGlobalVar not implemented", varName);
	}
    void Assembler::pushSharedVarReference(const char* varName)
	{
		RaiseError(0, "pushGlobalVar not implemented", varName);
	}

    void Assembler::pushConstant(int i)
    {
        if (i == 0)
            pushIntZero();
        else if (i == 1)
            pushIntOne();
        else {
            program.push_back(Instructions::iPushConstant);
            program.push_back(i);
        }
    }

    void Assembler::pushFloatConstant(float v)
    {
        union { int i; float f; } u;
        u.f = v;
        int i = u.i;
        program.push_back(Instructions::iPushFloatConstant);
        program.push_back(i);
    }

    void Assembler::pushRangedRandom(float r1, float r2) {
        pushFloatConstant(r2);
        pushFloatConstant(r1);
        program.push_back(Instructions::iRangedRandom);
    }

    void Assembler::popStore() {
        // compile instruction that pops the top of the eval stack & stores it in a variable.
        // the variable will be an index, which is used to get the offset in the memory block of variables for this machine
        program.push_back(Instructions::iPopStore);
    }

    void Assembler::pushIntOne()  { program.push_back(Instructions::iPushIntOne); }
    void Assembler::pushIntZero() { program.push_back(Instructions::iPushIntZero); }
    void Assembler::endState()    { program.push_back(Instructions::iStateEnd); }

    void Assembler::beginMachine(const char *name) {
        machineName.assign(name);
        stringIndex(name);    // register the name of the machine
        maxStateOrd = 0;
        maxStringOrd = 0;
        maxVarIndex = 0;
        maxSharedVarIndex = 0;
        stateAddrMap.clear();
        stateOrdinalMap.clear();
        stringOrdinalMap.clear();
        program.clear();
        varType.clear();
        varIndex.clear();
        sharedVarType.clear();
        sharedVarIndex.clear();
    }

    void Assembler::endMachine() {
        std::unique_ptr<Exemplar> e = createExemplar();
        if (e) {
            exemplars.push_back(std::move(e));
        }
    }

    void Assembler::subStateEnd() { program.push_back(Instructions::iSubStateEnd); }
    void Assembler::paramsStart() { program.push_back(Instructions::iParamsStart); }
    void Assembler::paramsEnd() { program.push_back(Instructions::iParamsEnd); }

    void Assembler::nop()
    {
        program.push_back(Instructions::iNop);
    }

    void Assembler::getGlobalVar()
    {
        program.push_back(Instructions::iGetGlobalVar);
    }

    void Assembler::pushSharedVar(const char* varName)
    {
        pushSharedVarIndex(varName);
        program.push_back(Instructions::iGetSharedVar);
    }

    void Assembler::pushInstanceVar(const char* varName)
    {
        int i = instanceVarIndex(varName);
        if (i >= 0)
            program.push_back(Instructions::iGetSelfVar | (i << 16));
        else {
            //RaiseError();
        }
    }

    void Assembler::opAdd()
    {
        program.push_back(Instructions::iOpAdd);
    }

    void Assembler::opSubtract()
    {
        program.push_back(Instructions::iOpSubtract);
    }

    void Assembler::opMultiply()
    {
        program.push_back(Instructions::iOpMultiply);
    }

    void Assembler::opDivide()
    {
        program.push_back(Instructions::iOpDivide);
    }

    void Assembler::opNegate()
    {
        program.push_back(Instructions::iOpNegate);
    }

    void Assembler::opModulus()
    {
        program.push_back(Instructions::iOpModulus);
    }

    void Assembler::opGreaterThan() {
        program.push_back(Instructions::iOpGreaterThan);
    }
    void Assembler::opLessThan() {
        program.push_back(Instructions::iOpLessThan);
    }

    void Assembler::pushLocalVar(const char* varName) {
        if (localVariables.empty())
            lcRaiseError("no local parameters", 0, 0);

        int i = localVariableIndex(varName);
        int e = i << 16;
        program.push_back(Instructions::iGetLocalVarObj | e);
    }

    void Assembler::beginForEach(const char* name, const char* type) {
        // for each will call Run as if executing substates
        // in the case of for body in bodies, type will be varobj and name will be body
        // the local parameters will be created, pushed, and cleaned up by the forEach instruction itself
        //
        localVariables.push_back(LocalVariable(name, type));
        program.push_back(Instructions::iForEach);
        int patch = gotoAddr();
        clauseStack.push_back(patch);
    }

    void Assembler::endForEach() {
        localVariables.pop_back();
        subStateEnd();
        int patch = clauseStack.back();
        clauseStack.pop_back();
        _patchGoto(patch);
    }

    void Assembler::beginOn() {
        // empty
    }

    void Assembler::beginOnStatements() {
        int patch = gotoAddr();
        clauseStack.push_back(patch);
    }

    void Assembler::endOnStatements() {
        int patch = clauseStack.back();
        clauseStack.pop_back();
        _patchGoto(patch);
        subStateEnd();
    }

    void Assembler::beginConditionalClause() {
        int patch = gotoAddr();
        clauseStack.push_back(patch);
    }

    void Assembler::beginContraConditionalClause() {
        int patch = clauseStack.back();
        clauseStack.pop_back();
        int patch2 = gotoAddr();
        _patchGoto(patch);
        clauseStack.push_back(patch2);
    }

    void Assembler::endConditionalClause() {
        int patch = clauseStack.back();
        clauseStack.pop_back();
        _patchGoto(patch);
    }

    int Assembler::gotoAddr()
    {
        program.push_back(Instructions::iGotoAddr);
        int patch = (int) program.size();
        program.push_back(0);	// patch will patch over this zero
        return patch;
    }

    void Assembler::gotoAddr(int addr)
    {
        program.push_back(Instructions::iGotoAddr);
        program.push_back(addr);
    }

    void Assembler::gotoState(const char* stateName)
    {
        int i = stateIndex(stateName);
        program.push_back(Instructions::iPushConstant);
        program.push_back(i);
        program.push_back(Instructions::iGotoState);
    }

    void Assembler::launchMachine()
    {
        program.push_back(Instructions::iLaunchMachine);
    }

    void Assembler::ifEq()
    {
        program.push_back(Instructions::iIfEq);
    }

    void Assembler::ifLte0()
    {
        program.push_back(Instructions::iIfLte0);
    }

    void Assembler::ifGte0()
    {
        program.push_back(Instructions::iIfGte0);
    }

    void Assembler::ifLt0()
    {
        program.push_back(Instructions::iIfLt0);
    }

    void Assembler::ifGt0()
    {
        program.push_back(Instructions::iIfGt0);
    }

    void Assembler::ifEq0()
    {
        program.push_back(Instructions::iIfEq0);
    }

    void Assembler::ifNotEq0()
    {
        program.push_back(Instructions::iIfNotEq0);
    }

    void Assembler::beginState(const char* name)
    {
        stringIndex(name);
        stateIndex(name);
        stateAddrMap[name] = (int) program.size();
    }

    void Assembler::addSharedVariable(const char* name, const char* type)
    {
        stringIndex(name);
        stringIndex(type);
        sharedVarType[name] = type;
        sharedVarIndex[name] = maxSharedVarIndex;
        ++maxSharedVarIndex;
    }
    void Assembler::addInstanceVariable(const char* name, const char* type)
    {
        stringIndex(name);
        stringIndex(type);
        varType[name] = type;
        varIndex[name] = maxVarIndex;
        ++maxVarIndex;
    }

    void Assembler::_patchGoto(int patch)
    {
        program[patch] = (int) program.size();
    }

    void Assembler::disassemble(const std::string& machineName, FILE* f)
    {
        fprintf(f, "\nMachine: %s\n", machineName.c_str());

        std::vector<std::string> strings(maxStringOrd);
        strings.resize(maxStringOrd);
        for (std::map<std::string, int>::const_iterator
             i = stringOrdinalMap.begin(); i != stringOrdinalMap.end(); ++i)
        {
            strings[i->second] = i->first;
        }

        fprintf(f, "\tStrings:\n");
        int index = 0;
        for (std::vector<std::string>::const_iterator
             i = strings.begin(); i != strings.end(); ++i)
        {
            fprintf(f, "\t\t%d:\"%s\"\n", index, (*i).c_str());
            ++index;
        }

        fprintf(f, "\tVariables:\n");
        for (std::map<std::string, int>::const_iterator
             i = varIndex.begin(); i != varIndex.end(); ++i)
        {
            std::map<std::string, std::string>::const_iterator j = varType.find(i->first);
            fprintf(f, "\t\t%d:%s \"%s\"\n", i->second, j->second.c_str(), i->first.c_str());
        }

        fprintf(f, "\tShared Variables:\n");
        for (std::map<std::string, int>::const_iterator
             i = sharedVarIndex.begin(); i != sharedVarIndex.end(); ++i)
        {
            std::map<std::string, std::string>::const_iterator j = sharedVarType.find(i->first);
            fprintf(f, "\t\t%d:%s \"%s\"\n", i->second, j->second.c_str(), i->first.c_str());
        }

        fprintf(f, "\tStates:\n");
        for (std::map<std::string, int>::const_iterator
             i = stateAddrMap.begin(); i != stateAddrMap.end(); ++i)
        {
            // note: it would be nice if this was in the same order as the disassembly
            fprintf(f, "\t\t%02d:[%04d]%s\n", stateIndex(i->first.c_str()), i->second, i->first.c_str());
        }

        int maxPC = (int) program.size();
        for (int i = 0; i < maxPC; ++i) {
            int skip;
            char const*const dis = Instructions::disassemble(&program[0], i, strings, skip);
            fprintf(f, "%s\n", dis);
            i += skip;
        }
    }

    int Assembler::programSize()
    {
        return (int) program.size();
    }

    std::unique_ptr<Exemplar> Assembler::createExemplar()
    {
        Exemplar* e = new Exemplar();
        int stateCount = maxStateOrd;
        e->stateTableLength = stateCount;

        unsigned int* states = (unsigned int*) malloc(sizeof(unsigned int) * stateCount);
        e->stateNameIndex = (int*) malloc(sizeof(int) * stateCount);
        for (int i = 0; i < stateCount; ++i) {
            std::map<std::string, int>::const_iterator it;
            for (it = stateOrdinalMap.begin(); it != stateOrdinalMap.end(); ++it) {
                if (it->second == i)
                    break;
            }
            if (it == stateOrdinalMap.end()) {
                lcRaiseError("Bad state index", 0, 0);
                return std::unique_ptr<Exemplar>(e);
            }
            std::map<std::string, int>::const_iterator it2 = stateAddrMap.find(it->first);
            if (it2 == stateAddrMap.end()) {
                lcRaiseError("Bad state name", 0, 0);
                return std::unique_ptr<Exemplar>(e);
            }
            states[i] = it2->second;
            e->stateNameIndex[i] = stringIndex(it->first.c_str());
        }
        e->stateTable = states;

        e->programStoreLength = (int) program.size();
        e->programStore = (unsigned int*)malloc(e->programStoreLength * sizeof(unsigned int));
        memcpy(e->programStore, (unsigned int*) &program[0], e->programStoreLength * sizeof(unsigned int));

        e->varCount = maxVarIndex;
        e->varNameIndex = (int*) malloc(sizeof(int) * maxVarIndex);
        e->varTypeIndex = (int*) malloc(sizeof(int) * maxVarIndex);
        for (int i = 0; i < maxVarIndex; ++i)
        {
            std::map<std::string, int>::const_iterator it;
            for (it = varIndex.begin(); it != varIndex.end(); ++it) {
                if (it->second == i)
                    break;
            }
            if (it == varIndex.end()) {

                for (it = varIndex.begin(); it != varIndex.end(); ++it) {
                    printf("%s:%d\n", it->first.c_str(), it->second);
                }

                lcRaiseError("Bad var index", 0, 0);
                return std::unique_ptr<Exemplar>(e);
            }
            e->varNameIndex[i] = stringIndex(it->first.c_str());
            std::map<std::string, std::string>::const_iterator it2 = varType.find(it->first);
            if (it2 == varType.end()) {
                lcRaiseError("Var type not found", 0, 0);
                return std::unique_ptr<Exemplar>(e);
            }
            e->varTypeIndex[i] = stringIndex(it2->second.c_str());
        }
        e->sharedVarCount = maxSharedVarIndex;
        e->sharedVarNameIndex = (int*) malloc(sizeof(int) * maxSharedVarIndex);
        e->sharedVarTypeIndex = (int*) malloc(sizeof(int) * maxSharedVarIndex);
        for (int i = 0; i < maxSharedVarIndex; ++i) {
            std::map<std::string, int>::const_iterator it;
            for (it = sharedVarIndex.begin(); it != sharedVarIndex.end(); ++it) {
                if (it->second == i)
                    break;
            }
            if (it == sharedVarIndex.end()) {
                lcRaiseError("Bad shared var index", 0, 0);
                return std::unique_ptr<Exemplar>(e);
            }
            e->sharedVarNameIndex[i] = stringIndex(it->first.c_str());
            std::map<std::string, std::string>::const_iterator it2 = sharedVarType.find(it->first);
            if (it2 == sharedVarType.end()) {
                lcRaiseError("Shared Var type not found", 0, 0);
                return std::unique_ptr<Exemplar>(e);
            }
            e->sharedVarTypeIndex[i] = stringIndex(it2->second.c_str());
        }

        e->stringArrayLength = maxStringOrd;

        int* stringStarts = (int*) malloc(sizeof(int) * maxStringOrd);
        e->stringArray = stringStarts;

        // reorder the strings into a simple array
        std::vector<std::string> stringTable;
        stringTable.resize(maxStringOrd);
        for (std::map<std::string, int>::const_iterator
             it = stringOrdinalMap.begin(); it != stringOrdinalMap.end(); ++it)
        {
            stringTable[it->second] = it->first;
        }

        // calculate space required to hold all the strings
        int stringTableLength = 0;
        for (int i = 0; i < e->stringArrayLength; ++i) {
            stringTableLength += (int) stringTable[i].length() + 1;
        }
        e->stringDataLength = stringTableLength;

        char* stringData = stringTableLength ? (char*) malloc(sizeof(char) * stringTableLength) : 0;
        e->stringData = stringData;

        int currOff = 0;
        char* curr = const_cast<char*>(e->stringData);
        for (size_t i = 0; i < stringTable.size(); ++i) {
            strcpy(curr, stringTable[i].c_str());
            e->stringArray[i] = currOff;
            currOff += (int) strlen(stringTable[i].c_str()) + 1;
            curr = const_cast<char*>(&e->stringData[currOff]);
        }

        for (size_t i = 0; i < stringTable.size(); ++i) {
            const char* test = &e->stringData[e->stringArray[i]];
            if (!strcmp(machineName.c_str(), test)) {
                e->nameStr = test;	///< Pointer to 0th string, convenience for identifying the Exemplar in the debugger
                break;
            }
        }


        return std::unique_ptr<Exemplar>(e);
    }

	void Assembler::addGlobal(const char* name, const char* type) {
		RaiseError(0, "global not implemented ", name);
	}

    void Assembler::addGlobalBson(const char* name, shared_ptr<Lab::Bson> b) {
		RaiseError(0, "global bson not implemented ", name);
    }

	void Assembler::addGlobalString(const char* name, const char* value) {
		RaiseError(0, "global string not implemented ", name);
	}
	void Assembler::addGlobalInt(const char* name, int value) {
		RaiseError(0, "global int not implemented ", name);
	}
	void Assembler::addGlobalFloat(const char* name, float value) {
		RaiseError(0, "global float not implemented ", name);
	}


} // Landru


EXTERNC void landruAssembleProgram(void* rootNode)
{
    using namespace Landru;
    ASTNode* program = (ASTNode*) rootNode;
    if (program->token == Landru::kTokenProgram) {
        CompilationContext c;
        c.compile(program);
    }
}

EXTERNC void landruAssembleProgramToRuntime(void* rootNode, void* runtime,
                                            std::vector<std::pair<std::string, Json::Value*> >* jsonVars)
{
    using namespace Landru;
    ASTNode* program = (ASTNode*) rootNode;
    if (program->token == Landru::kTokenProgram) {
        CompilationContext c;
        c.compile(program);
        c.copyToRuntime((Landru::Engine*) runtime, jsonVars);
    }
}




