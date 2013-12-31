
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruVM/Fiber.h"

#include "LandruStd/StringVarObj.h"
#include "LandruVM/Continuation.h"
#include "LandruVM/Engine.h"
#include "LandruVM/Exemplar.h"
#include "LandruVM/Instructions.h"
#include "LandruVM/MachineCache.h"
#include "LandruVM/Stack.h"
#include "LandruVM/StackPrint.h"
#include "LandruVM/VarObj.h"
#include "LandruVM/VarObjArray.h"
#include "LandruVM/RaiseError.h"
#include "LandruVM/VarObjStackUtil.h"


#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>


//#define LVERBOSE
#ifdef LVERBOSE
const bool verboseTrace = true;
# define TRACE(f, pc, op) printf("%d:%s\n", pc, Instructions::opName(op));
#else
const bool verboseTrace = false;
# define TRACE(f, pc, op)
#endif

namespace Landru {

	namespace {

	
        template <typename T, typename TChar>
        class Splitter
        {
        public:
            Splitter(const T& src, TChar splitter) {
                count = 0;
                TChar* curr = buffer;
                parts[count++] = curr;
                const TChar* srcPtr = src.c_str();
                while (*srcPtr) {
                    if (*srcPtr != splitter) {
                        *curr++ = *srcPtr++;
                    }
                    else {
                        *curr++ = '\0';
                        parts[count++] = curr;
                        ++srcPtr;
                    }
                }
                *curr++ = '\0'; // terminate final string
                parts[count] = 0;
            }
            
            ~Splitter()
            {
            }
            
            TChar*  parts[64];       // max parts
            TChar   buffer[520];     // MAX_PATH
            int     count;
        };
        
        std::vector<std::string> Split(const std::string& input, char splitter, bool escapes, bool empties)
        {
            std::vector<std::string> output;
            if (input.find(splitter) == std::string::npos) {
                output.push_back(input);
            }
            else {
                size_t curr = 0;
                size_t start = 0;
                size_t end = input.length();
                while (curr < end) {
                    if (escapes && input[curr] == '\\') {
                        ++curr;
                        if (curr != end && input[curr] == splitter)
                            ++curr;
                    }
                    else {
                        if (input[curr] == splitter) {
                            if (curr>start) {
                                output.push_back(input.substr(start, curr-start));
                            }
                            else if (empties && curr == 0) {
                                output.push_back("");
                            }
                            start = curr+1;
                            if (empties && (input[start] == splitter || start == end)) {
                                output.push_back("");
                                ++start;
                                ++curr;
                            }
                        }
                        ++curr;
                    }
                }
                if (curr - start > 0)
                    output.push_back(input.substr(start, curr-start));
            }
            
            
            return output;
        }
        
    
    }
	
    VarObjPtr* Fiber::main = 0;


    std::set<Fiber*>& Fiber::ActiveFibers()
    {
        static std::set<Fiber*> fibers;
        return fibers;
    }
    
    
    void Fiber::Create(VarPool* varpool, int maxStackDepth, int maxVars, VarObjPtr* self, const MachineCacheEntry* mce_)
    {
        _suspendedPC = -1;
        mce = mce_;
		
        _vars = varpool;
		for (int i = 0; i < mce->exemplar->varCount; ++i) {
			const char* varType = &mce->exemplar->stringData[mce->exemplar->stringArray[mce->exemplar->varTypeIndex[i]]];
			const char* varName = &mce->exemplar->stringData[mce->exemplar->stringArray[mce->exemplar->varNameIndex[i]]];
			VarObjPtr* v = VarObj::Factory(_vars, this, i, varType, varName);
			if (v == 0)
				RaiseError(0, "Unknown variable type", varType);
		}
        
        if (!strcmp(mce->exemplar->name(), "main"))
            main = self;

        resolvedFunctionArray = (FnRuntime*) malloc(sizeof(FnRuntime) * mce->exemplar->stringArrayLength);
        memset(resolvedFunctionArray, 0, sizeof(FnRuntime) * mce->exemplar->stringArrayLength);
        
        ActiveFibers().insert(this);
    }

    Fiber::~Fiber()
    {
        std::set<Fiber*>& fibers = ActiveFibers();
        std::set<Fiber*>::iterator i = fibers.find(this);
        if (i != fibers.end())
            fibers.erase(i);
    }




















    int Fiber::findFunction(FnRuntime* result,
                            VarObjPtr* self, 
                            Engine* engine, const char* funcName)
    {
        if (!result)
            return -1;

        VarObjPtr* libObj;
        int index = findFunctionS(self, engine, funcName, &libObj);

        if (libObj && index >= 0) {
            result->ref = new VarObjWeakRef(libObj);
            result->fn = libObj->vo->Func(index);
        }
        
        return index;
    }




    int Fiber::findFunctionS(VarObjPtr* self, Engine* engine,
                            const char* funcName,
                            VarObjPtr** libObj)
    {
        if (!self)
            return 0;
        
        std::vector<std::string> parts = Split(funcName, '.', false, false);
        return findFunctionR(self, engine, parts, 0, libObj);
    }
    
    // a function can be on the fiber, on the main VO, or on some variable
    enum FnSelf { FnSelfFiber, FnSelfMain, FnSomeVariable };
    
    // once we know what it is on, the fn might be on a shared var on whatever it is,
    // a local var on whatever it is, or on whatever it is itself.
    enum FnLocations {
        FnSomeSharedVarOnSelf, FnSomeLocalVarOnSelf, FnOnSelf
    };
    
    // so how do I look the silly thing up?
    // main and self are easy. some variable can be arbitrarily deep. how can I breadcrumb to it?
    
    
    // findFunction expects fiberToRun and libObj to be initialized before call
    // so that they can be incrementally overridden during recursion
    int Fiber::findFunctionR(VarObjPtr* self, Engine* engine, 
                             std::vector<std::string>& parts, int partIndex, 
                             VarObjPtr** libObj)
    {        
        VarPool* selfVars = self->vo->varPool();
        
        std::string& part = parts[partIndex];
        
        if (part == "$") {
            RaiseError(0, "Can't precompile stacktop function", "findFunction");
            return -1;
        }
        
        if (part == "main") {
            return findFunctionR(main, engine, parts,
                                 partIndex + 1, libObj);
        }

        // is it a shared var on the current fiber?
        Fiber* f = dynamic_cast<Fiber*>(self->vo);
        if (f) {
            VarObjPtr* vop = selfVars->varObj(f->mce->exemplar, part.c_str());
            if (vop) {
                *libObj = vop;
                return vop->vo->Func(parts[partIndex+1].c_str());
            }
        }
        
        // is it a local var?
        VarObjPtr* vop = selfVars->varObj(self->vo, part.c_str());
        if (vop) {
            if ((partIndex + 1) < parts.size())
                return findFunctionRV(vop, engine, parts,
                                      partIndex + 1, libObj);
            else {
                int fn = vop->vo->Func(part.c_str());
                if (fn >= 0) {
                    *libObj = vop;
                    return fn;
                }
                RaiseError(0, "Couldn't find function", part.c_str());
                *libObj = 0;
                return 0;
            }
        }
        
        // Is a function resolvable on this?
        int fn = self->vo->Func(part.c_str());
        if (fn >= 0) {
            *libObj = self;
            return fn;
        }

        // global variable?
        vop = engine->Global(part.c_str());
        if (vop) {
            return findFunctionRV(vop, engine, parts, partIndex + 1, libObj);
        }
        
        // attempt to resolve on engine directly
        return findFunctionRV(engine->self(), engine, parts, partIndex + 1, libObj);
    }
    
    // findFunction expects libObj to be initialized before call
    // so that it can be incrementally overridden during recursion
    int Fiber::findFunctionRV(VarObjPtr* self, Engine* engine, 
                              std::vector<std::string>& parts, int partIndex, 
                              VarObjPtr** libObj)
    {
        std::string& part = parts[partIndex];
        
        if (!self) {
            RaiseError(0, "function unresolved", part.c_str());
            return -1;
        }
        
        VarPool* selfVars = self->vo->varPool();
        
        // is it a local var?
        VarObjPtr* vop = selfVars->varObj(self->vo, part.c_str());
        if (vop)
            return findFunctionRV(vop, engine, parts, partIndex + 1, libObj);

        // Is a function resolvable on self?
        int fn = self->vo->Func(part.c_str());
        if (fn >= 0) {
            *libObj = self;
            return fn;
        }

        RaiseError(0, "function unresolved", part.c_str());
        return -1;
    }



    Fiber::RunEndCondition Fiber::Run(Engine* engine,
                                      VarObjPtr* self,
                                      float elapsedTime,
                                      int pc,
                                      LStack* stack,
                                      Continuation* contextContinuation,
                                      VarObjArray* exeStack,
                                      VarObjArray* locals)
	{
        RunEndCondition retval = kStateEnd;
		_suspendedPC = -1;
                
        unsigned int* programStore = mce->exemplar->programStore;
        
		bool stateEnd = false;
		do {
            unsigned int instruction = programStore[pc];
			Instructions::Op op = (Instructions::Op) (instruction &0xffff);
            
            //printf("%s: %d [%d]\n", this->exemplar->name, pc, LStackDepth(stack));
            
            TRACE(this, pc, op);
            
			switch (op) {
			case Instructions::iLaunchMachine:
				{
                    // get the parameters off the top of the stack
                    // the zeroeth element of the parameters is the name of the machine to launch
                    VarObjArray* localVoa;
                    Pop<VarObjArray> t1(stack, _vars, localVoa);
                    if (!localVoa)
                        RaiseError(pc, "no params for launching machine", "iLaunchMachine");
                    
                    VarObjStrongRef ref(localVoa->get(0));
                    StringVarObj* svo = dynamic_cast<StringVarObj*>(ref.vo->vo);
                    if (!svo)
                        RaiseError(pc, "no machine named to launch", "iLaunchMachine");
                    
                    localVoa->pop_front();
                    
                    if (svo) {                        
                        const char* machineName = svo->getCstr();
                        VarObjPtr* newF = engine->machineCache()->createFiber(machineName); // comes with one strong ref already
                        if (newF) {
/*                            LStackPushParamMark(stack);
                            int locals = localVoa->size();
                            for (int i = 1; i < locals; ++i)
                                LStackPushVarObj(stack, (LVarPool*) _vars, (LVarObj*) localVoa->get(i));
                            Fiber* f2 = (Fiber*) newF->vo;
                            f2->Run(engine, newF, elapsedTime, f2->EntryPoint(), stack, 0, exeStack);
                            for (int i = 1; i < locals; ++i)
                                LStackPop(stack, (LVarPool*) _vars); // pop the params mark and the locals
                            _vars->releaseStrongRef(newF); */
                            Fiber* f2 = (Fiber*) newF->vo;
                            f2->Run(engine, newF, elapsedTime, f2->EntryPoint(), stack, 0, exeStack, localVoa);
                            _vars->releaseStrongRef(newF);
                        }
                        else {
                            printf("Couldn't launch machine %s\n", machineName);
                        }
                    }
                    ++pc;
				}
				break;

			case Instructions::iPushConstant:
				{
					int data = programStore[++pc];
                    pushInt(stack, _vars, data);
					++pc;
				}
				break;

			case Instructions::iPushIntOne:
				{
                    pushInt(stack, _vars, 1);
					++pc;
				}
				break;

			case Instructions::iPushIntZero:
				{
                    pushInt(stack, _vars, 0);
					++pc;
				}
				break;

			case Instructions::iPushFloatConstant:
				{
					union {
						int i;
						float f;
					} data;
					data.i = programStore[++pc];
                    pushReal(stack, _vars, data.f);
					++pc;
				}
				break;

			case Instructions::iPopStore:
				{
                    // the current use of this instruction is after a machine is created, its pointer is on the stack
					int varIndex = popInt(stack, _vars);		// get var index
                    VarObjStrongRef vp((VarObjPtr*) LStackTopVarObj(stack, (LVarPool*) _vars));
                    VarObj* var = vp.vo->vo; 
                    LStackPop(stack, (LVarPool*) _vars);		// get data to store there
					if (varIndex >= mce->exemplar->varCount)
						_vars->setSlot(mce, varIndex - mce->exemplar->varCount, var, true);
					else
						_vars->setSlot(this, varIndex, var, true);			// store the varobj pointer in its slot
					++pc;
				}
				break;

            case Instructions::iCreateTempString:
                {
                    int index = LStackTopStringIndex(stack, (LVarPool*) _vars); LStackPop(stack, (LVarPool*) _vars);
                    const char* str = &mce->exemplar->stringData[mce->exemplar->stringArray[index]];
                    if (verboseTrace)
                        printf("\t\ttemp string: %s\n", str);
                    
                    VarObjPtr* var = StringVarObj::createString(_vars, "temp", str);
                    LStackPushVarObj(stack, (LVarPool*) _vars, (LVarObj*) var);
                    _vars->releaseStrongRef(var); // createString added a strong ref, so did push
                    ++pc;
                }
                break;
                    
            case Instructions::iParamsStart:
                {
                    LStackPushParamMark(stack);
                    ++pc;
                }
                break;
                    
            case Instructions::iParamsEnd:
                {
                    LStackFinalizeParamMark(stack, (LVarPool*) _vars);
                    ++pc;
                }
                break;

#if 0
            case Instructions::iOnMessage:
				{
					// +3 points to statement within the message response scope
					_engine->RegisterOnMessage(this, stack, pc + 3);
					++pc;
				}
				break;

			case Instructions::iOnTick:
				{
					// +3 points to statement within the tick response scope
					_engine->RegisterTick(this, pc + 3);
					++pc;
				}
				break;
#endif
            case Instructions::iIfEq:
                {
					// auto-promotes ints to floats for comparison.
					// if can't promote to float, an error is raised;
					/// @TODO provide comparisons for non numerics as well
					++pc;
					float val1 = popReal(stack, _vars);
					float val2 = popReal(stack, _vars);
                    
					// do float comparison
					/// @TODO, subtract, abs, and test for < eps?
                    if (val1 == val2)
                        pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;

            case Instructions::iIfLte0:
                {
					++pc;
					float val = popReal(stack, _vars);
					if (val <= 0)
						pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;

            case Instructions::iIfGte0:
                {
					++pc;
					float val = popReal(stack, _vars);
					if (val >= 0)
						pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;

            case Instructions::iIfLt0:
                {
					++pc;
					float val = popReal(stack, _vars);
					if (val < 0)
						pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;

            case Instructions::iIfGt0:
                {
					++pc;
					float val = popReal(stack, _vars);
					if (val > 0)
						pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;

            case Instructions::iIfEq0:
                {
					++pc;
					float val = popReal(stack, _vars);
					/// @TODO test for abs <= eps
					if (val == 0)
						pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;

            case Instructions::iIfNotEq0:
                {
					++pc;
					float val = popReal(stack, _vars);
					if (val != 0)
						pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;
                    
            case Instructions::iOpAdd:
                {
                    ++pc;
					float v1 = popReal(stack, _vars);
					float v2 = popReal(stack, _vars);
                    pushReal(stack, _vars, v1 + v2);
                }
                break;
                    
            case Instructions::iOpSubtract:
                {
                    ++pc;
					float v1 = popReal(stack, _vars);
					float v2 = popReal(stack, _vars);
                    pushReal(stack, _vars, v2 - v1);
                }
                break;

            case Instructions::iOpMultiply:
                {
                    ++pc;
					float v1 = popReal(stack, _vars);
					float v2 = popReal(stack, _vars);
                    pushReal(stack, _vars, v1 * v2);
                }
                break;
                    
            case Instructions::iOpDivide:
                {
                    ++pc;
					float v1 = popReal(stack, _vars);
					float v2 = popReal(stack, _vars);
                    pushReal(stack, _vars, v2 / v1);
                }
                break;
                    
            case Instructions::iOpNegate:
                {
                    ++pc;
					float v1 = popReal(stack, _vars);
                    pushReal(stack, _vars, -v1);
                }
                break;
                    
            case Instructions::iOpModulus:
                {
                    ++pc;
					float v1 = popReal(stack, _vars);
					float v2 = popReal(stack, _vars);
                    pushReal(stack, _vars, float(int(v2) % int(v1)));
                }
                break;
                    
                    
                    
            case Instructions::iRepeatBegin:
                {
                    ++pc;
                    
                    VarObjArray* localVoa;
                    Pop<VarObjArray> t1(stack, _vars, localVoa);
                    float val = localVoa->getReal(-1);
                    
                    pushInt(stack, _vars, 123456);
                    pushInt(stack, _vars, (int) val);
                }
                break;

            case Instructions::iRepeatTest:
                {
                    ++pc;
					int val = popInt(stack, _vars) - 1;
                    pushInt(stack, _vars, val);

                    if (val >= 0)        // if still positive, skip the goto
                        pc += 2;
                }
                break;

            case Instructions::iRepeatEnd:
                {
                    ++pc;
                    popInt(stack, _vars);
                    int data = popInt(stack, _vars);

                    if (data != 123456)
                        RaiseError(pc-1, "stack depth changed during repeat", 0);
                }
                break;
                    
            case Instructions::iForEach:
                {
                    ++pc; // skip forEach instruction
                    ++pc; // skip goto
                    int nextPC = programStore[pc];
                    
                    int loopTop = ++pc;
                    
                    VarObjStrongRef vp((VarObjPtr*) LStackTopVarObj(stack, engine->lvarPool()));
                    LStackPop(stack, (LVarPool*) _vars);
                    
                    Landru::VarObjArray forLocals("locals");
                    
                    VarObjArray* voa = dynamic_cast<VarObjArray*>(vp.vo->vo);
                    for (int i = 0; i < voa->size(); ++i) {
                        forLocals.add(voa->get(i));
#ifdef LVERBOSE
                        printf("\n\n\nForEach %d of %d -->>>>>> Start Run\n", i, voa->size());
#endif
                        Run(engine, self, elapsedTime, loopTop, stack, contextContinuation, exeStack, &forLocals);
                        forLocals.pop();
#ifdef LVERBOSE
                        printf("<<<<<<-- ForEach %d of %d End Run\n\n\n", i, voa->size());
#endif
                    }
                    pc = nextPC;
                }
                break;

            case Instructions::iGotoState:
				{
					int data = popInt(stack, _vars);
					pc = mce->exemplar->stateTable[data];
                    engine->ClearContinuations(self);
				}
				break;

			case Instructions::iGotoAddr:
				pc = programStore[++pc];
				break;

			case Instructions::iStateEnd:
				++pc;
				stateEnd = true;
                retval = kStateEnd;
				break;

			case Instructions::iSubStateEnd:
				++pc;
				stateEnd = true;
                retval = kSubStateEnd;
				break;

            case Instructions::iStateSuspend:
                ++pc;
				_suspendedPC = pc;
                stateEnd = true;
                retval = kStateSuspend;
                break;

			case Instructions::iRangedRandom:
				{
					float minF = popReal(stack, _vars);
					float maxF = popReal(stack, _vars);
					float r = (float) rand() / RAND_MAX;
					float range = maxF - minF;
					r *= range;
					r += minF;
                    pushReal(stack, _vars, r);
					++pc;
				}
				break;
					
























            case Instructions::iOnLibEvent:
				{
					int func = instruction >> 16;
                    if (verboseTrace) {
                        const char* funcName = &mce->exemplar->stringData[mce->exemplar->stringArray[func]];
                        printf("\t\ton lib %s\n", funcName);
                    }

                    FuncPtr fn = resolvedFunctionArray[func].fn;
                    if (!fn) {
                        const char* funcName = &mce->exemplar->stringData[mce->exemplar->stringArray[func]];
                        findFunction(&resolvedFunctionArray[func], self, engine, funcName);
                        fn = resolvedFunctionArray[func].fn;
                    }
                    VarObjPtr* vop = resolvedFunctionArray[func].ref->vo;
                    if (fn && vop && vop->vo) {
                        FnParams p;
                        p.parentContinuation = contextContinuation;
                        p.continuationPC = pc + 3; // the only line different from calling a function.
                        p.pc = pc;
                        p.vop = vop;
                        p.vo = vop->vo;
                        p.f = this;
                        p.self = self;
                        p.stack = stack;
                        p.engine = engine;
                        fn(&p); // if function could be resolved, call it
                    }
                    ++pc;
                }
                break;

                case Instructions::iCallFunction:
				{
					int func = instruction >> 16;
       
                    if (verboseTrace) {
                        const char* funcName = &mce->exemplar->stringData[mce->exemplar->stringArray[func]];
                        printf("\t\tcall %s\n", funcName);
                    }

                    FuncPtr fn = resolvedFunctionArray[func].fn;
                    if (!fn) {
                        const char* funcName = &mce->exemplar->stringData[mce->exemplar->stringArray[func]];
                        findFunction(&resolvedFunctionArray[func], self, engine, funcName);
                        fn = resolvedFunctionArray[func].fn;
                    }
                    VarObjPtr* vop = resolvedFunctionArray[func].ref->vo;
                    if (fn && vop && vop->vo) {
                        FnParams p;
                        p.parentContinuation = contextContinuation;
                        p.pc = pc;
                        p.vop = vop;
                        p.vo = vop->vo;
                        p.f = this;
                        p.self = self;
                        p.stack = stack;
                        p.engine = engine;
                        fn(&p); // if function could be resolved, call it
                    }
                    ++pc;
                }
                break;
                    
                case Instructions::iDotChain:
                {
                    VarObjStrongRef vp((VarObjPtr*) LStackTopVarObj(stack, engine->lvarPool()));
                    exeStack->push_back(vp.vo);
                    LStackPop(stack, engine->lvarPool());
                    ++pc;
                }
                break;

                case Instructions::iDynamicCallLibFunction:
				{
					int func = instruction >> 16;
				    const char* funcName = &mce->exemplar->stringData[mce->exemplar->stringArray[func]];
                    if (verboseTrace)
                        printf("\t\tdynamic call %s\n", funcName);

                    if (exeStack->empty())
                        RaiseError(pc, "No object on execution stack", "iDynamicCallLibFunction");
                    
                    VarObjStrongRef vp(exeStack->top());
                    exeStack->pop();
                    
                    VarObjPtr* libObj = 0;
                    int fnIndex = findFunctionS(vp.vo, engine, funcName, &libObj);
                    
                    if (self && libObj) {
                        VarObj::FuncPtr fn = libObj->vo->Func(fnIndex);
                        if (fn >= 0) {
                            FnParams p;
                            p.parentContinuation = contextContinuation;
                            p.pc = pc;
                            p.vop = libObj;
                            p.vo = libObj->vo;
                            p.f = this;
                            p.self = self;
                            p.stack = stack;
                            p.engine = engine;
                            fn(&p); // if function could be resolved, call it
                        }
                        else
                            RaiseError(pc, "Unknown function", "");
                    }
                    else
                        RaiseError(pc, "Unknown object", "");
                    
                    ++pc;
				}
                break;

            case Instructions::iGetSharedVar:
                {
                    int index = popInt(stack, _vars);
                    
                    // is it a local var on the main fiber?
  					if (-1 != index)
					{
                        VarObjPtr* vo = _vars->varObj(mce, index);
                        LStackPushVarObj(stack, (LVarPool*) _vars, (LVarObj*) vo);
					}
                    else
                    {
                        RaiseError(pc, "variable not found on main fiber", 0);
                    }
                    ++pc;
                }
                break;

            case Instructions::iGetGlobalVar:
                {
                    int index = LStackTopStringIndex(stack, (LVarPool*) _vars); LStackPop(stack, (LVarPool*) _vars);
                    const char* varName = &mce->exemplar->stringData[mce->exemplar->stringArray[index]];
                    std::vector<std::string> parts = Split(varName, '.', false, false);
                    
                    VarObjPtr* vo = engine->Global(parts[0].c_str());
                    if (!vo)
                        RaiseError(pc, "Can't find global variable", parts[0].c_str());
                    
                    if (parts.size() > 1) {
                        vo = vo->vo->GetVar(parts[1].c_str());
                        if (!vo)
                            RaiseError(pc, "Can't find global variable", varName);
                    }
                    
                    LStackPushVarObj(stack, (LVarPool*) _vars, (LVarObj*) vo);
                    ++pc;
                }
                break;
                    
			case Instructions::iGetSelfVar:
				{
					int varIndex = LStackTopStringIndex(stack, (LVarPool*) _vars); LStackPop(stack, (LVarPool*) _vars);
                    LVarObj* lvo = (LVarObj*) GetVar(varIndex);
                    if (!lvo) {
                        if (varIndex > mce->exemplar->varCount)
                            RaiseError(pc, "Unknown variable", 0);
                        else {
                            int varNameIndex = mce->exemplar->varNameIndex[varIndex];
                            RaiseError(pc, "Can't find self var", &mce->exemplar->stringData[mce->exemplar->stringArray[varNameIndex]]);
                        }
                    }

					LStackPushVarObj(stack, (LVarPool*) _vars, lvo);
					++pc;
				}
				break;
                    
            case Instructions::iGetLocalString:  // probably need to deprecate get local string
            case Instructions::iGetLocalVarObj:
                {
					int paramIndex = instruction >> 16;
                    LStackPushVarObj(stack, (LVarPool*) _vars, (LVarObj*) locals->get(paramIndex));
                    ++pc;
                }
                break;
                    
            case Instructions::iGetLocalExeVarObj:
                {
                    int paramIndex = instruction >> 16;
                    exeStack->add(locals->get(paramIndex));
                    ++pc;
                }
                break;
                    
			default:
				++pc;
				break;
			}
            
            if (verboseTrace)
                stackPrint(stack);
		}
		while (!stateEnd);

        return retval;
	}

	LANDRU_DECL_FN(Fiber, add)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

		float val1 = voa->getReal(-1);
		float val2 = voa->getReal(-2);
        pushReal(p, val1 + val2);
	}

    LANDRU_DECL_FN(Fiber, sub)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);
        
		float val1 = voa->getReal(-1);
		float val2 = voa->getReal(-2);
        pushReal(p, val2 - val1);
	}

    LANDRU_DECL_FN(Fiber, mul)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);
        
		float val1 = voa->getReal(-1);
		float val2 = voa->getReal(-2);
        pushReal(p, val1 * val2);
	}

    LANDRU_DECL_FN(Fiber, div)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);
        
		float val1 = voa->getReal(-1);
		float val2 = voa->getReal(-2);
        pushReal(p, val2 / val1);
	}

	LANDRU_DECL_FN(Fiber, min)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);
        
		float val1 = voa->getReal(-1);
		float val2 = voa->getReal(-2);
        pushReal(p, val1 < val2 ? val1 : val2);
    }

	LANDRU_DECL_FN(Fiber, max)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);
        
		float val1 = voa->getReal(-1);
		float val2 = voa->getReal(-2);
        pushReal(p, val2 < val1 ? val1 : val2);
   }

    LANDRU_DECL_FN(Fiber, int)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);
        
		float val1 = voa->getReal(-1);
        pushInt(p, val1);
    }

	LANDRU_DECL_FN(Fiber, sqrt)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);
        
		float val1 = voa->getReal(-1);
        pushReal(p, sqrtf(val1));
    }

    LANDRU_DECL_FN(Fiber, toggle)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);
        
		int val1 = voa->getInt(-1);
        pushInt(p, 1 - val1);
    }

//	LANDRU_DECL_FN(Fiber, send)
//    {
//		int message = LStackTopInt(stack); LStackPop(stack);
//		const char* messageString = &f->exemplar->stringData[f->exemplar->stringArray[message]];
//		const char* machineTypeNameString = f->TopString(stack); LStackPop(stack);
//		f->_engine->SendMessageToAll(machineTypeNameString, messageString);
//	}

	LANDRU_DECL_FN(Fiber, new)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);
        
        StringVarObj* svo = dynamic_cast<StringVarObj*>(voa->get(-1)->vo);

        if (!svo)
            return;
        
        const char* newType = svo->getCstr();
		VarObjPtr* v = VarObj::Factory(p->engine->varPool(), p->engine, -1, newType, "new");
        if (v == 0)
            RaiseError(p->pc, "Unknown variable type", newType);

		LStackPushVarObj(p->stack, p->engine->lvarPool(), (LVarObj*) v);
	}


    LANDRU_DECL_TABLE_BEGIN(Fiber)
        LANDRU_DECL_ENTRY(Fiber, add)
        LANDRU_DECL_ENTRY(Fiber, sub)
        LANDRU_DECL_ENTRY(Fiber, mul)
        LANDRU_DECL_ENTRY(Fiber, div)
        LANDRU_DECL_ENTRY(Fiber, sqrt)
        LANDRU_DECL_ENTRY(Fiber, min)
        LANDRU_DECL_ENTRY(Fiber, max)
        LANDRU_DECL_ENTRY(Fiber, toggle)
//        LANDRU_DECL_ENTRY(Fiber, send)
        LANDRU_DECL_ENTRY(Fiber, new)
        LANDRU_DECL_ENTRY(Fiber, int)
    LANDRU_DECL_TABLE_END(Fiber)

} // Landru



