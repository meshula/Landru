
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT


#include "LandruAssembler.h"
#include "LandruVM/Exemplar.h"
#include "LandruVM/Instructions.h"
#include "LandruCompiler/lcRaiseError.h"
#include "LandruStd/JsonVarObj.h"
#include "LabJson/bson.h"

#include "LandruCompiler/AST.h"
#include "LabText/itoa.h"
#include "LabText/TextScanner.hpp"
#include "LandruVM/Engine.h"
#include "LandruVM/VarObj.h"
#include "LandruVM/MachineCache.h"


//#define ASSEMBLER_TRACE(a) printf("Assembler: %s\n", #a)
#define ASSEMBLER_TRACE(a)

using namespace std;

namespace Landru {



    class CompilationContext
    {
    public:

        ~CompilationContext()
        {
            // also get rid of bson objects
            for (std::map<std::string, bson*>::const_iterator i = globals.begin(); i != globals.end(); ++i)
            {
                bson_destroy(i->second);
                free(i->second);
            }
        }

        bson* compileBson(Landru::ASTNode* rootNode)
        {
            using namespace Landru;
            bsonArrayNesting.clear();
            bsonCurrArrayIndex = 0;
            bson* b = (bson*) malloc(sizeof(bson));
            bson_init(b);
            compileBsonData(rootNode, b);
            bson_finish(b);
            bson_print(b);
            return b;
        }

        void compileBsonData(Landru::ASTNode* rootNode, bson* b)
        {
            using namespace Landru;
            for (ASTConstIter i = rootNode->children.begin(); i != rootNode->children.end(); ++i)
                compileBsonDataElement(*i, b);
        }

        void compileBsonDataElement(Landru::ASTNode* rootNode, bson* b)
        {
            using namespace Landru;
            switch(rootNode->token) {

                case kTokenDataObject:
                {
                    ASSEMBLER_TRACE(kTokenDataObject);
                    if (bsonArrayNesting.size() > 0) {
                        char indexStr[16];
                        itoa(bsonCurrArrayIndex, indexStr, 10);
                        //printf("----> %s\n", indexStr);
                        bson_append_start_object(b, indexStr);
                        compileBsonData(rootNode, b); // recurse
                        bson_append_finish_object(b);
                        ++bsonCurrArrayIndex;
                    }
                    else
                        compileBsonData(rootNode, b); // recurse
                    break;
                }

                case kTokenDataArray:
                {
                    ASSEMBLER_TRACE(kTokenDataArray);
                    bsonArrayNesting.push_back(bsonCurrArrayIndex);
                    bsonCurrArrayIndex = 0;
                    compileBsonData(rootNode, b); // recurse
                    bsonCurrArrayIndex = bsonArrayNesting.back();
                    bsonArrayNesting.pop_back();
                    break;
                }

                case kTokenDataElement:
                {
                    ASSEMBLER_TRACE(kTokenDataElement);
                    ASTConstIter i = rootNode->children.begin();
                    if ((*i)->token == kTokenDataIntLiteral) {
                        int val;
                        atoi((*i)->str2.c_str());
                        bson_append_int(b, rootNode->str2.c_str(), val);
                    }
                    else if ((*i)->token == kTokenDataFloatLiteral)
                    {
                        float val = atof(rootNode->str2.c_str());
                        bson_append_double(b, rootNode->str2.c_str(), val);
                    }
                    else if ((*i)->token == kTokenDataNullLiteral) {
                        bson_append_null(b, rootNode->str2.c_str());
                    }
                    else {
                        bson_append_start_object(b, rootNode->str2.c_str());
                        compileBsonData(rootNode, b); // recurse
                        bson_append_finish_object(b);
                    }
                    break;
                }

                case kTokenDataFloatLiteral:
                {
                    ASSEMBLER_TRACE(kTokenDataFloatLiteral);
                    float val = atof(rootNode->str2.c_str());
                    bson_append_double(b, rootNode->str2.c_str(), val);
                    break;
                }

                case kTokenDataIntLiteral:
                {
                    ASSEMBLER_TRACE(kTokenDataIntLiteral);
                    int val = atoi(rootNode->str2.c_str());
                    bson_append_int(b, rootNode->str2.c_str(), val);
                    break;
                }

                case kTokenDataNullLiteral:
                {
                    ASSEMBLER_TRACE(kTokenDataNullLiteral);
                    bson_append_null(b, rootNode->str2.c_str());
                    break;
                }

                case kTokenDataStringLiteral:
                {
                    ASSEMBLER_TRACE(kTokenDataStringLiteral);
                    bson_append_string(b, rootNode->str2.c_str(), rootNode->str1.c_str());
                    break;
                }

                default:
                    break;
            }
        }

        void addGlobalRequire(const char* name, const char* module)
        {
            requires[name] = module;
            requiresList.push_back(name);
        }
        int requiresIndex(const char* name) {
            string n(name);
            int i = 0;
            for (auto s : requiresList) {
                if (s == n) return i;
                ++i;
            }
            i = 0;
            for (auto s : requiresList) {
                auto j = requires.find(s);
                if (j == requires.end())
                    continue;
                if (j->second == n)
                    return i;
                ++i;
            }
            return -1;
        }


        void addGlobalBson(const char* name, bson* b)
        {
            globals[name] = b;
        }

        void compileGlobalVariable(Landru::ASTNode* rootNode)
        {
            using namespace Landru;
            ASTConstIter kind = rootNode->children.begin();
            if ((*kind)->token == kTokenRequire) {
                addGlobalRequire(rootNode->str2.c_str(), (*kind)->str2.c_str());
            }
            else {
                addGlobalBson(rootNode->str2.c_str(), compileBson(*kind));
            }
        }

        void compileMachine(Landru::ASTNode* rootNode)
        {
            Landru::Assembler a(this);
            a.assemble(rootNode);
            exemplars.push_back(a.exemplars.front());
        }

        void compile(Landru::ASTNode* programNode)
        {
            for (ASTConstIter i = programNode->children.begin(); i != programNode->children.end(); ++i) {
                if ((*i)->token == kTokenGlobalVariable)
                    compileGlobalVariable(*i);
                else if ((*i)->token == kTokenMachine)
                    compileMachine(*i);
            }
        }

        void copyToRuntime(Landru::Engine* e, std::vector<std::pair<std::string, Json::Value*> >* jsonVars)
        {
            for (int i = 0; i < requiresList.size(); ++i) {
                e->AddRequire(requiresList[i].c_str(), i);
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

        map<string, string> requires;
        vector<string> requiresList;
        
        map<string, bson*> globals;
        vector<shared_ptr<Landru::Exemplar>> exemplars;
        
        // temp for bson parsing
        std::vector<int> bsonArrayNesting;
        int bsonCurrArrayIndex;
        
    }; // CompilationContext
    


	
    Assembler::Assembler(CompilationContext* cc)
    : context(cc)
    {
        program.reserve(1000);
        startAssembling();
    }

    Assembler::~Assembler()
    {
    }

    bool Assembler::isGlobalVariable(const char* name)
    {
        string s(name);
        if (context->globals.find(s) != context->globals.end())
            return true;
        
        return (strlen(name) > 5 && !strncmp(name, "main.", 5));
    }

    bool Assembler::isRequire(const char* name) {
        return (context->requires.find(string(name)) != context->requires.end());
    }

    int Assembler::requireIndex(const char* name) {
        string s(name);
        for (int i = 0; i < context->requiresList.size(); ++i) {
            if (s == context->requiresList[i])
                return i;
        }
        return -1;
    }

    bool Assembler::isSharedVariable(const char* name)
    {
        std::string s(name);
        return (sharedVarType.find(s) != sharedVarType.end());
    }

    bool Assembler::isLocalParam(const char* name)
    {
        return localParamIndex(name) >= 0;
    }
    
    int Assembler::localParamIndex(const char* name)
    {
        if (localParameters.empty())
            return -1;
        
        // parameters were parsed in order into the vector;
        // they are indexed backwards from the local stack frame, so if the parameters were a, b, c
        // the runtime indices are a:2 b:1 c:0
        // search from most recently pushed because of scoping, eg in this case
        // for x in range(): for y in range(): local real x ...... ; ;
        // the innermost x should be found within the y loop, and the outermost in the x loop
        int m = localParameters.size() - 1;
        for (int i = m; i >= 0; --i)
            if (!strcmp(name, localParameters[i].name.c_str()))
                return m - i;
        return -1;
    }

    void Assembler::addLocalParam(const char* name, const char* type) {
        localParameters.push_back(LocalParameter(name, type));
    }
    
    void Assembler::beginLocalVariableScope() {
        localVariableState.push_back(0);
    }
    
    void Assembler::addLocalVariable(const char *name,  const char* type) {
        addLocalParam(name, type);
        pushStringConstant(name);
        pushStringConstant(type);
        callFactory();
        localVariableState[localVariableState.size()-1] += 1;
    }
    
    void Assembler::endLocalVariableScope() {
        int localsCount = localVariableState[localVariableState.size()-1];
        localVariableState.pop_back();
        
        for (int i = 0; i < localsCount; ++i) {
            popLocal();
            localParamPop();
        }
    }
    
    void Assembler::localParamPop() {
        localParameters.pop_back();
    }

    void Assembler::popLocal() {
        program.push_back(Instructions::iPopLocal);
    }
    
    void Assembler::callFactory() {
        program.push_back(Instructions::iFactory);
    }

        
    void Assembler::startAssembling()
    {
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
        
    void Assembler::finalizeAssembling()
    {
        std::unique_ptr<Exemplar> e = createExemplar();
        if (e) {
            exemplars.push_back(std::move(e));
            
            //FILE* f2 = fopen("/Users/dp/newasm2.txt", "wt");
            //e->disassemble(f2);
            //fflush(f2);
            //fclose(f2);
            //f2 = 0;
        }
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
        char* dot = strchr(fnName, '.');
        if (dot) {
            vector<string> parts = TextScanner::Split(string(fnName), string("."));
            int index = 0;

            // a dotted function will either be on a local parameter, a require, or an a local variable, or on a class shared variable

            int i = localParamIndex(parts[index].c_str());
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
                        if (isSharedVariable(parts[index].c_str())) {
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
        program.push_back(Instructions::iDotChain);
    }

    void Assembler::getRequire(int i) {
        int index = i << 16;
        program.push_back(index | Instructions::iGetRequire);
    }

    void Assembler::pushStringConstant(const char* str)
    {
        pushConstant(stringIndex(str));
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

    void Assembler::storeToVar(const char *varName) {
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
        
    void Assembler::pushGlobalVarIndex(const char* varName)
    {
        // global variables will be fetched at run time by name from main
        pushConstant(stringIndex(varName)); 
    }

    void Assembler::pushConstant(int i)
    {
        program.push_back(Instructions::iPushConstant);
        program.push_back(i);
    }

    void Assembler::pushFloatConstant(float v)
    {
        union { int i; float f; } u;
        u.f = v;
        int i = u.i;
        program.push_back(Instructions::iPushFloatConstant);
        program.push_back(i);
    }
        
    void Assembler::rangedRandom() {
        program.push_back(Instructions::iRangedRandom);
    }

    void Assembler::popStore() {
        // compile instruction that pops the top of the eval stack & stores it in a variable.
        // the variable will be an index, which is used to get the offset in the memory block of variables for this machine
        program.push_back(Instructions::iPopStore);
    }

    void Assembler::pushIntOne()  { program.push_back(Instructions::iPushIntOne); }
    void Assembler::pushIntZero() { program.push_back(Instructions::iPushIntZero); }
    void Assembler::stateEnd()    { program.push_back(Instructions::iStateEnd); }

    void Assembler::subStateEnd()
    {
        program.push_back(Instructions::iSubStateEnd);
    }

    void Assembler::paramsStart()
    {
        program.push_back(Instructions::iParamsStart);
    }

    void Assembler::paramsEnd()
    {
        program.push_back(Instructions::iParamsEnd);
    }

    void Assembler::nop()
    {
        program.push_back(Instructions::iNop);
    }
        
    void Assembler::getGlobalVar()
    {
        program.push_back(Instructions::iGetGlobalVar);
    }

    void Assembler::getSharedVar()
    {
        program.push_back(Instructions::iGetSharedVar);
    }

    void Assembler::getSelfVar(int index)
    {
        program.push_back(Instructions::iGetSelfVar | (index << 16));
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

    void Assembler::getLocalParam(int i)
    {
        if (localParameters.empty())
            lcRaiseError("no local parameters", 0, 0);
        
        int e = i << 16;
        program.push_back(Instructions::iGetLocalVarObj | e);
    }
        
    void Assembler::beginForEach(const char* name, const char* type) {
        // for each will call Run as if executing substates
        // in the case of for body in bodies, type will be varobj and name will be body
        // the local parameters will be created, pushed, and cleaned up by the forEach instruction itself
        //
        addLocalParam(name, type);
        program.push_back(Instructions::iForEach);
        int patch = gotoAddr();
        clauseStack.push_back(patch);
    }
    
    void Assembler::endForEach() {
        localParamPop();
        subStateEnd();
        int patch = clauseStack.back();
        clauseStack.pop_back();
        _patchGoto(patch);
    }

    void Assembler::beginOn() {
        int patch = gotoAddr();
        clauseStack.push_back(patch);
    }
    
    void Assembler::endOn() {
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

    void Assembler::_addState(const char* name)
    {
        stateIndex(name);
        stateAddrMap[name] = (int) program.size();
    }

    void Assembler::_addVariable(const char* name, const char* type, bool shared)
    {
        stringIndex(name);
        stringIndex(type);
        if (shared) {
            sharedVarType[name] = type;
            sharedVarIndex[name] = maxSharedVarIndex;
            ++maxSharedVarIndex;
        }
        else {
            varType[name] = type;
            varIndex[name] = maxVarIndex;
            ++maxVarIndex;
        }
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
            stringTableLength += stringTable[i].length() + 1;
        }
        e->stringDataLength = stringTableLength;
        
        char* stringData = stringTableLength ? (char*) malloc(sizeof(char) * stringTableLength) : 0;
        e->stringData = stringData;
        e->nameStr = e->stringData;	///< Pointer to 0th string, convenience for identifying the Exemplar in the debugger
        
        int currOff = 0;
        char* curr = const_cast<char*>(e->stringData);
        for (size_t i = 0; i < stringTable.size(); ++i) {
            strcpy(curr, stringTable[i].c_str());
            e->stringArray[i] = currOff;
            currOff += strlen(stringTable[i].c_str()) + 1;
            curr = const_cast<char*>(&e->stringData[currOff]);
        }
        
        return std::unique_ptr<Exemplar>(e);
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




