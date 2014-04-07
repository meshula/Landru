
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruVM/Fiber.h"

#include "LandruStd/StringVarObj.h"
#include "LandruVM/Continuation.h"
#include "LandruVM/Engine.h"
#include "LandruVM/Exemplar.h"
#include "LandruVM/GeneratorVarObj.h"
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
# define VERBOSE(fmt, ...) printf(fmt, __VA_ARGS__)
#else
const bool verboseTrace = false;
# define VERBOSE(fmt, args...)
# define TRACE(f, pc, op)
#endif

namespace Landru {

    using namespace std;

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
	
    
    void Fiber::Create(VarObjFactory* factory, int maxStackDepth, int maxVars, std::shared_ptr<MachineCacheEntry> mce_)
    {
        _suspendedPC = -1;
        mce = mce_;

        if (mce->exemplar->varCount > 0)
            _vars = unique_ptr<VarObjArray>(new VarObjArray(""));

		for (int i = 0; i < mce->exemplar->varCount; ++i) {
			const char* varType = &mce->exemplar->stringData[mce->exemplar->stringArray[mce->exemplar->varTypeIndex[i]]];
			const char* varName = &mce->exemplar->stringData[mce->exemplar->stringArray[mce->exemplar->varNameIndex[i]]];
            std::shared_ptr<VarObj> v(factory->make(varType, varName));
			if (!v)
				RaiseError(0, "Unknown variable type", varType);
            _vars->add(v);
		}
    }

    Fiber::~Fiber()
    {
    }






    Fiber::RunEndCondition Fiber::Run(Engine* engine,
                                      std::shared_ptr<Fiber> self,
                                      float elapsedTime,
                                      int pc,
                                      LStack* stack,
                                      Continuation* contextContinuation,
                                      std::vector<std::shared_ptr<Fiber>>& exeStack,
                                      VarObjArray* locals)
	{
        if (!locals) {
            locals = _vars.get();
        }
        RunEndCondition retval = kStateEnd;
		_suspendedPC = -1;
                
        unsigned int* programStore = mce->exemplar->programStore;
        
		bool stateEnd = false;
		do {
            unsigned int instruction = programStore[pc];
			Instructions::Op op = (Instructions::Op) (instruction &0xffff);
            
            //printf("%d [%d]\n", pc, LStackDepth(stack));
            
            TRACE(this, pc, op);
            
			switch (op) {
			case Instructions::iLaunchMachine:
				{
                    // get the parameters off the top of the stack
                    // the zeroeth element of the parameters is the name of the machine to launch
                    std::shared_ptr<VarObjArray> voa = stack->top<VarObjArray>();
                    stack->pop();
                    VarObjArray* localVoa = voa.get();
                    if (!localVoa)
                        RaiseError(pc, "no params for launching machine", "iLaunchMachine");
                    
                    StringVarObj* svo = dynamic_cast<StringVarObj*>(voa->get(0).lock().get());
                    if (!svo)
                        RaiseError(pc, "no machine named to launch", "iLaunchMachine");
                    
                    localVoa->pop_front();
                    
                    if (svo) {                        
                        const char* machineName = svo->getCstr();
                        std::shared_ptr<Fiber> newF = engine->machineCache()->createFiber(engine->factory(), machineName); // comes with one strong ref already
                        if (newF) {
/*                            LStackPushParamMark(stack);
                            int locals = localVoa->size();
                            for (int i = 1; i < locals; ++i)
                                LStackPushVarObj(stack, (LVarPool*) _vars, (LVarObj*) localVoa->get(i));
                            Fiber* f2 = (Fiber*) newF->vo;
                            f2->Run(engine, newF, elapsedTime, f2->EntryPoint(), stack, 0, exeStack);
                            for (int i = 1; i < locals; ++i)
                                stack->pop(); // pop the params mark and the locals
                            _vars->releaseStrongRef(newF); */
                            newF->Run(engine, newF, elapsedTime, newF->EntryPoint(), stack, 0, exeStack, localVoa);
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
                    pushInt(stack, data);
					++pc;
				}
				break;

			case Instructions::iPushIntOne:
				{
                    pushInt(stack, 1);
					++pc;
				}
				break;

			case Instructions::iPushIntZero:
				{
                    pushInt(stack, 0);
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
                    pushReal(stack, data.f);
					++pc;
				}
				break;

			case Instructions::iPopStore:
				{
                    // the current use of this instruction is after a machine is created, its pointer is on the stack
					int varIndex = popInt(stack);		// get var index
                    std::shared_ptr<VarObj> vp = stack->top<VarObj>();
                    stack->pop();
					if (varIndex >= mce->exemplar->varCount)
                        mce->vars[varIndex - mce->exemplar->varCount] = vp;
					else
                        _vars->set(vp, varIndex);
					++pc;
				}
				break;

            case Instructions::iCreateTempString:
                {
                    int index = LStackTopStringIndex(stack);
                    stack->pop();
                    const char* str = &mce->exemplar->stringData[mce->exemplar->stringArray[index]];
                    if (verboseTrace)
                        printf("\t\ttemp string: %s\n", str);
                    
                    stack->push(StringVarObj::createString("temp", str));
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
                    LStackFinalizeParamMark(stack);
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
					float val1 = popReal(stack);
					float val2 = popReal(stack);
                    
					// do float comparison
					/// @TODO, subtract, abs, and test for < eps?
                    if (val1 == val2)
                        pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;

            case Instructions::iIfLte0:
                {
					++pc;
					float val = popReal(stack);
					if (val <= 0)
						pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;

            case Instructions::iIfGte0:
                {
					++pc;
					float val = popReal(stack);
					if (val >= 0)
						pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;

            case Instructions::iIfLt0:
                {
					++pc;
					float val = popReal(stack);
					if (val < 0)
						pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;

            case Instructions::iIfGt0:
                {
					++pc;
					float val = popReal(stack);
					if (val > 0)
						pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;

            case Instructions::iIfEq0:
                {
					++pc;
					float val = popReal(stack);
					/// @TODO test for abs <= eps
					if (val == 0)
						pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;

            case Instructions::iIfNotEq0:
                {
					++pc;
					float val = popReal(stack);
					if (val != 0)
						pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                }
                break;
                    
            case Instructions::iOpAdd:
                {
                    ++pc;
					float v1 = popReal(stack);
					float v2 = popReal(stack);
                    pushReal(stack, v1 + v2);
                }
                break;
                    
            case Instructions::iOpSubtract:
                {
                    ++pc;
					float v1 = popReal(stack);
					float v2 = popReal(stack);
                    pushReal(stack, v2 - v1);
                }
                break;

            case Instructions::iOpMultiply:
                {
                    ++pc;
					float v1 = popReal(stack);
					float v2 = popReal(stack);
                    pushReal(stack, v1 * v2);
                }
                break;
                    
            case Instructions::iOpDivide:
                {
                    ++pc;
					float v1 = popReal(stack);
					float v2 = popReal(stack);
                    pushReal(stack, v2 / v1);
                }
                break;
                    
            case Instructions::iOpNegate:
                {
                    ++pc;
					float v1 = popReal(stack);
                    pushReal(stack, -v1);
                }
                break;
                    
            case Instructions::iOpModulus:
                {
                    ++pc;
					float v1 = popReal(stack);
					float v2 = popReal(stack);
                    pushReal(stack, float(int(v2) % int(v1)));
                }
                break;
            
            case Instructions::iForEach:
                {
                    ++pc; // skip forEach instruction
                    ++pc; // skip goto
                    int nextPC = programStore[pc];
                    int loopTop = ++pc;

                    std::shared_ptr<VarObjArray> vp = stack->top<VarObjArray>();
                    stack->pop();

                    VarObjArray* voa = vp.get();
                    if (voa->size() > 0) {
                        Landru::GeneratorVarObj* gen = dynamic_cast<Landru::GeneratorVarObj*>(voa->get(0).lock().get());
                        if (gen) {
                            for (gen->begin(voa); !gen->done(); gen->next()) {
                                Landru::VarObjArray forLocals("locals");
                                gen->generate(forLocals);
                                VERBOSE("\n\n\nForEach in generator -->>>>>> Start Run\n");
                                Run(engine, self, elapsedTime, loopTop, stack, contextContinuation, exeStack, &forLocals);
                                forLocals.pop();
                                VERBOSE("<<<<<<-- ForEach in generator End Run\n\n\n");
                            }
                        }
                        else {
                            Landru::VarObjArray forLocals("locals");
                            for (int i = 0; i < voa->size(); ++i) {
                                forLocals.add(voa->get(i).lock());
                                VERBOSE("\n\n\nForEach %d of %d -->>>>>> Start Run\n", i, voa->size());
                                Run(engine, self, elapsedTime, loopTop, stack, contextContinuation, exeStack, &forLocals);
                                forLocals.pop();
                                VERBOSE("<<<<<<-- ForEach %d of %d End Run\n\n\n", i, voa->size());
                            }
                        }
                    }
                    pc = nextPC;
                }
                break;

            case Instructions::iGotoState:
				{
					int data = popInt(stack);
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
					float minF = popReal(stack);
					float maxF = popReal(stack);
					float r = (float) rand() / RAND_MAX;
					float range = maxF - minF;
					r *= range;
					r += minF;
                    pushReal(stack, r);
					++pc;
				}
				break;
					

















                case Instructions::iGetRequire: {
                    shared_ptr<VarObj> vo = engine->Require(instruction >> 16);
                    if (!!vo)
                        stack->push(vo);
                    else
                        RaiseError(pc, "COuldn't find required library", 0);
                    ++pc; }
                    break;



                case Instructions::iCallFunction: {
					int func = instruction >> 16;

                    // top of stack must be the thing to call the function on
                    std::shared_ptr<VarObj> vp = stack->top<VarObj>();
                    stack->pop();

                    if (vp) {
                        if (verboseTrace) {
                            const char* funcName = &mce->exemplar->stringData[mce->exemplar->stringArray[func]];
                            printf("\t\tcall %s\n", funcName);
                        }

                        FuncPtr fn = 0;
                        const char* funcName = &mce->exemplar->stringData[mce->exemplar->stringArray[func]];
                        int funcIndex = vp->Func(funcName);
                        if (funcIndex >= 0)
                            fn = vp->Func(funcIndex);
                        if (fn) {
                            FnParams p;
                            p.parentContinuation = contextContinuation;
                            p.pc = pc;
                            p.vo = vp;
                            p.f = self;
                            p.stack = stack;
                            p.engine = engine;
                            p.continuationPC = p.pc + 3;    // skip the goto in the case of conditionals
                            fn(&p); // if function could be resolved, call it
                        }
                    }
                    ++pc; }
                    break;
                    
                case Instructions::iDotChain: {
                    exeStack.push_back(stack->top<Fiber>());
                    stack->pop();
                    ++pc; }
                    break;

                case Instructions::iGetSharedVar: {
                    int index = popInt(stack);

                    // is it a local var on the main fiber?
  					if (-1 != index && index < mce->vars.size())
                        stack->push(mce->vars[index]);
                    else
                        RaiseError(pc, "variable not found on main fiber", 0);
                    ++pc; }
                    break;

                case Instructions::iGetGlobalVar: {
                    int index = LStackTopStringIndex(stack);
                    stack->pop();
                    const char* varName = &mce->exemplar->stringData[mce->exemplar->stringArray[index]];
                    std::vector<std::string> parts = Split(varName, '.', false, false);

                    std::shared_ptr<VarObj> vo = engine->Global(parts[0].c_str()).lock();
                    if (!vo)
                        RaiseError(pc, "Can't find global variable", parts[0].c_str());

                    if (parts.size() > 1) {
                        vo = vo->GetVar(parts[1].c_str()).lock();
                        if (!vo)
                            RaiseError(pc, "Can't find global variable", varName);
                    }

                    stack->push(vo);
                    ++pc; }
                    break;

                case Instructions::iGetSelfVar: {
					int varIndex = LStackTopStringIndex(stack);
                    stack->pop();
                    std::shared_ptr<VarObj> lvo = GetVar(varIndex).lock();
                    if (!lvo) {
                        if (varIndex > mce->exemplar->varCount)
                            RaiseError(pc, "Unknown variable", 0);
                        else {
                            int varNameIndex = mce->exemplar->varNameIndex[varIndex];
                            RaiseError(pc, "Can't find self var", &mce->exemplar->stringData[mce->exemplar->stringArray[varNameIndex]]);
                        }
                    }

                    stack->push(lvo);
					++pc; }
                    break;

                case Instructions::iGetLocalString:  // probably need to deprecate get local string
                case Instructions::iGetLocalVarObj: {
					int paramIndex = instruction >> 16;
                    stack->push(locals->get(paramIndex).lock());
                    ++pc; }
                    break;

                case Instructions::iGetVarFromVar: {
                    shared_ptr<VarObj> vo = LStackTopVarObj(stack).lock();
                    int func = instruction >> 16;
                    shared_ptr<VarObj> varVo = vo->GetVar(mce->exemplar->varNameIndex[func]).lock();
                    stack->push(varVo);
                    ++pc; }
                    break;

                case Instructions::iGetLocalExeVarObj: {
                    int paramIndex = instruction >> 16;
                    std::shared_ptr<Fiber> f = std::dynamic_pointer_cast<Fiber>(locals->get(paramIndex).lock());
                    if (!f)
                        RaiseError(pc, "No fiber on stack", 0);
                    exeStack.push_back(f);
                    ++pc; }
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
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		float val1 = voa->getReal(-1);
		float val2 = voa->getReal(-2);
        pushReal(p, val1 + val2);
	}

    LANDRU_DECL_FN(Fiber, sub)
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		float val1 = voa->getReal(-1);
		float val2 = voa->getReal(-2);
        pushReal(p, val2 - val1);
	}

    LANDRU_DECL_FN(Fiber, mul)
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		float val1 = voa->getReal(-1);
		float val2 = voa->getReal(-2);
        pushReal(p, val1 * val2);
	}

    LANDRU_DECL_FN(Fiber, div)
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		float val1 = voa->getReal(-1);
		float val2 = voa->getReal(-2);
        pushReal(p, val2 / val1);
	}

	LANDRU_DECL_FN(Fiber, min)
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		float val1 = voa->getReal(-1);
		float val2 = voa->getReal(-2);
        pushReal(p, val1 < val2 ? val1 : val2);
    }

	LANDRU_DECL_FN(Fiber, max)
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		float val1 = voa->getReal(-1);
		float val2 = voa->getReal(-2);
        pushReal(p, val2 < val1 ? val1 : val2);
   }

    LANDRU_DECL_FN(Fiber, int)
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		float val1 = voa->getReal(-1);
        pushInt(p, val1);
    }

	LANDRU_DECL_FN(Fiber, sqrt)
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		float val1 = voa->getReal(-1);
        pushReal(p, sqrtf(val1));
    }

    LANDRU_DECL_FN(Fiber, toggle)
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		int val1 = voa->getInt(-1);
        pushInt(p, 1 - val1);
    }

//	LANDRU_DECL_FN(Fiber, send)
//    {
//		int message = LStackTopInt(stack); stack->pop();
//		const char* messageString = &f->exemplar->stringData[f->exemplar->stringArray[message]];
//		const char* machineTypeNameString = f->TopString(stack);  stack->pop();
//		f->_engine->SendMessageToAll(machineTypeNameString, messageString);
//	}

	LANDRU_DECL_FN(Fiber, new)
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

        StringVarObj* svo = dynamic_cast<StringVarObj*>(voa->get(-1).lock().get());

        if (!svo)
            return;
        
        const char* newType = svo->getCstr();
        std::shared_ptr<VarObj> v = p->engine->factory()->make(newType, "new");
        if (!v)
            RaiseError(p->pc, "Unknown variable type", newType);

        p->stack->push(v);
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



