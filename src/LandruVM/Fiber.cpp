
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
        _sharedVars = mce->sharedVars;
    }

    Fiber::~Fiber()
    {
    }






    Fiber::RunEndCondition Fiber::Run(RunContext* rc) {
        VarObjArray* locals = rc->locals;
        if (!locals)
            locals = _vars.get();

        RunEndCondition retval = kStateEnd;
		_suspendedPC = -1;
                
        unsigned int* programStore = mce->exemplar->programStore;
        
		bool stateEnd = false;
		do {
            unsigned int instruction = programStore[rc->pc];
			Instructions::Op op = (Instructions::Op) (instruction &0xffff);
            
            //printf("%d [%d]\n", pc, LStackDepth(stack));
            
            TRACE(this, pc, op);

			switch (op) {
                case Instructions::iLaunchMachine: {
                    // get the parameters off the top of the stack
                    // the zeroeth element of the parameters is the name of the machine to launch
                    std::shared_ptr<VarObjArray> voa = rc->stack->top<VarObjArray>();
                    rc->stack->pop();
                    VarObjArray* localVoa = voa.get();
                    if (!localVoa)
                        RaiseError(rc->pc, "no params for launching machine", "iLaunchMachine");

                    StringVarObj* svo = dynamic_cast<StringVarObj*>(voa->get(0).lock().get());
                    if (!svo)
                        RaiseError(rc->pc, "no machine named to launch", "iLaunchMachine");

                    localVoa->pop_front();

                    if (svo) {
                        const char* machineName = svo->getCstr();
                        std::shared_ptr<Fiber> newF = rc->engine->machineCache()->createFiber(rc->engine->factory(), machineName); // comes with one strong ref already
                        if (newF) {
                            RunContext newRc;
                            newRc.engine = rc->engine;
                            newRc.fiber = newF;
                            newRc.elapsedTime = rc->elapsedTime;
                            newRc.pc = newF->EntryPoint();
                            newRc.stack = rc->stack;
                            newRc.continuationContext = 0;
                            newRc.locals = localVoa;
                            newF->Run(&newRc);
                        }
                        else {
                            printf("Couldn't launch machine %s\n", machineName);
                        }
                    }
                    ++rc->pc; }
                    break;

                case Instructions::iPushConstant: {
					int data = programStore[++rc->pc];
                    pushInt(rc->stack, data);
					++rc->pc; }
                    break;

                case Instructions::iPushIntOne: {
                    pushInt(rc->stack, 1);
					++rc->pc; }
                    break;

                case Instructions::iPushIntZero: {
                    pushInt(rc->stack, 0);
					++rc->pc; }
                    break;

                case Instructions::iPushFloatConstant: {
					union {
						int i;
						float f;
					} data;
					data.i = programStore[++rc->pc];
                    pushReal(rc->stack, data.f);
					++rc->pc; }
                    break;

                case Instructions::iPopStore: {
                    // the current use of this instruction is after a machine is created, its pointer is on the stack
					int varIndex = popInt(rc->stack);		// get var index
                    std::shared_ptr<VarObj> vp = rc->stack->top<VarObj>();
                    rc->stack->pop();
                    _vars->set(vp, varIndex);
					++rc->pc; }
                    break;

                case Instructions::iCreateTempString: {
                    int index = LStackTopStringIndex(rc->stack);
                    rc->stack->pop();
                    const char* str = &mce->exemplar->stringData[mce->exemplar->stringArray[index]];
                    if (verboseTrace)
                        printf("\t\ttemp string: %s\n", str);

                    rc->stack->push(StringVarObj::createString("temp", str));
                    ++rc->pc; }
                    break;

                case Instructions::iFactory: {
                    int nameIndex = LStackTopStringIndex(rc->stack);
                    rc->stack->pop();
                    int typeNameIndex = LStackTopStringIndex(rc->stack);
                    rc->stack->pop();
                    const char* varType = &mce->exemplar->stringData[mce->exemplar->stringArray[typeNameIndex]];
                    const char* varName = &mce->exemplar->stringData[mce->exemplar->stringArray[nameIndex]];
                    std::shared_ptr<VarObj> v(rc->engine->factory()->make(varType, varName));
                    rc->stack->push(v);
                    ++rc->pc; }
                    break;
                    
                case Instructions::iParamsStart: {
                    LStackPushParamMark(rc->stack);
                    ++rc->pc; }
                    break;

                case Instructions::iParamsEnd: {
                    LStackFinalizeParamMark(rc->stack);
                    ++rc->pc; }
                    break;

                case Instructions::iIfEq: {
					// auto-promotes ints to floats for comparison.
					// if can't promote to float, an error is raised;
					/// @TODO provide comparisons for non numerics as well
					++rc->pc;
					float val1 = popReal(rc->stack);
					float val2 = popReal(rc->stack);

					// do float comparison
					/// @TODO, subtract, abs, and test for < eps?
                    if (val1 == val2)
                        rc->pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                } break;

                case Instructions::iIfLte0: {
					++rc->pc;
					float val = popReal(rc->stack);
					if (val <= 0)
						rc->pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                } break;

                case Instructions::iIfGte0: {
					++rc->pc;
					float val = popReal(rc->stack);
					if (val >= 0)
						rc->pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                } break;

                case Instructions::iIfLt0: {
					++rc->pc;
					float val = popReal(rc->stack);
					if (val < 0)
						rc->pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                } break;

                case Instructions::iIfGt0: {
					++rc->pc;
					float val = popReal(rc->stack);
					if (val > 0)
						rc->pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                } break;

                case Instructions::iIfEq0: {
					++rc->pc;
					float val = popReal(rc->stack);
					/// @TODO test for abs <= eps
					if (val == 0)
						rc->pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                } break;

                case Instructions::iIfNotEq0: {
					++rc->pc;
					float val = popReal(rc->stack);
					if (val != 0)
						rc->pc += 2;    // skip to body of if (otherwise, next instruction is goto end of substate)
                } break;
                    
                case Instructions::iOpAdd: {
                    ++rc->pc;
					float v1 = popReal(rc->stack);
					float v2 = popReal(rc->stack);
                    pushReal(rc->stack, v1 + v2);
                } break;
                    
            case Instructions::iOpSubtract: {
                    ++rc->pc;
					float v1 = popReal(rc->stack);
					float v2 = popReal(rc->stack);
                    pushReal(rc->stack, v2 - v1);
                }
                break;

            case Instructions::iOpMultiply: {
                    ++rc->pc;
					float v1 = popReal(rc->stack);
					float v2 = popReal(rc->stack);
                    pushReal(rc->stack, v1 * v2);
                }
                break;
                    
                case Instructions::iOpDivide: {
                    ++rc->pc;
					float v1 = popReal(rc->stack);
					float v2 = popReal(rc->stack);
                    pushReal(rc->stack, v2 / v1);
                } break;

                case Instructions::iOpNegate: {
                    ++rc->pc;
					float v1 = popReal(rc->stack);
                    pushReal(rc->stack, -v1);
                } break;
                    
                case Instructions::iOpModulus: {
                    ++rc->pc;
					float v1 = popReal(rc->stack);
					float v2 = popReal(rc->stack);
                    pushReal(rc->stack, float(int(v2) % int(v1)));
                } break;
            
                case Instructions::iForEach: {
                    ++rc->pc; // skip forEach instruction
                    ++rc->pc; // skip goto
                    int nextPC = programStore[rc->pc];
                    int loopTop = ++rc->pc;

                    std::shared_ptr<VarObjArray> vp = rc->stack->top<VarObjArray>();
                    rc->stack->pop();

                    VarObjArray* voa = vp.get();
                    if (voa->size() > 0) {
                        Landru::GeneratorVarObj* gen = dynamic_cast<Landru::GeneratorVarObj*>(voa->get(0).lock().get());
                        if (gen) {
                            for (gen->begin(voa); !gen->done(); gen->next()) {
                                Landru::VarObjArray forLocals("locals");
                                gen->generate(forLocals);
                                VERBOSE("\n\n\nForEach in generator -->>>>>> Start Run\n");

                                RunContext newRc;
                                newRc.engine = rc->engine;
                                newRc.fiber = rc->fiber;
                                newRc.elapsedTime = rc->elapsedTime;
                                newRc.pc = loopTop;
                                newRc.stack = rc->stack;
                                newRc.continuationContext = rc->continuationContext;
                                newRc.locals = &forLocals;
                                Run(&newRc);
                                //Run(engine, self, elapsedTime, loopTop, stack, contextContinuation, &forLocals);
                                forLocals.pop();
                                VERBOSE("<<<<<<-- ForEach in generator End Run\n\n\n");
                            }
                        }
                        else {
                            Landru::VarObjArray forLocals("locals");
                            for (int i = 0; i < voa->size(); ++i) {
                                forLocals.add(voa->get(i).lock());
                                VERBOSE("\n\n\nForEach %d of %d -->>>>>> Start Run\n", i, voa->size());

                                RunContext newRc;
                                newRc.engine = rc->engine;
                                newRc.fiber = rc->fiber;
                                newRc.elapsedTime = rc->elapsedTime;
                                newRc.pc = loopTop;
                                newRc.stack = rc->stack;
                                newRc.continuationContext = rc->continuationContext;
                                newRc.locals = &forLocals;
                                Run(&newRc);
                                //Run(engine, self, elapsedTime, loopTop, stack, contextContinuation, &forLocals);
                                forLocals.pop();
                                VERBOSE("<<<<<<-- ForEach %d of %d End Run\n\n\n", i, voa->size());
                            }
                        }
                    }
                    rc->pc = nextPC;
                } break;

            case Instructions::iGotoState: {
					int data = popInt(rc->stack);
					rc->pc = mce->exemplar->stateTable[data];
                    rc->engine->ClearContinuations(rc->fiber);
				}
				break;

			case Instructions::iGotoAddr:
				rc->pc = programStore[++rc->pc];
				break;

			case Instructions::iStateEnd:
				++rc->pc;
				stateEnd = true;
                retval = kStateEnd;
				break;

			case Instructions::iSubStateEnd:
				++rc->pc;
				stateEnd = true;
                retval = kSubStateEnd;
				break;

            case Instructions::iStateSuspend:
                ++rc->pc;
				_suspendedPC = rc->pc;
                stateEnd = true;
                retval = kStateSuspend;
                break;

                case Instructions::iRangedRandom: {
					float minF = popReal(rc->stack);
					float maxF = popReal(rc->stack);
					float r = (float) rand() / RAND_MAX;
					float range = maxF - minF;
					r *= range;
					r += minF;
                    pushReal(rc->stack, r);
					++rc->pc;
				}
                    break;

                case Instructions::iGetRequire: {
                    shared_ptr<VarObj> vo = rc->engine->Require(instruction >> 16);
                    if (!!vo)
                        rc->stack->push(vo);
                    else
                        RaiseError(rc->pc, "COuldn't find required library", 0);
                    ++rc->pc; }
                    break;



                case Instructions::iCallFunction: {
					int func = instruction >> 16;

                    // top of stack must be the thing to call the function on
                    std::shared_ptr<VarObj> vp = rc->stack->top<VarObj>();
                    rc->stack->pop();

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
                            p.contextContinuation = rc->continuationContext;
                            p.pc = rc->pc;
                            p.vo = vp;
                            p.fiber = rc->fiber;
                            p.stack = rc->stack;
                            p.engine = rc->engine;
                            p.continuationPC = p.pc + 3;    // skip the goto in the case of conditionals
                            fn(&p); // if function could be resolved, call it
                        }
                    }
                    ++rc->pc; }
                    break;
                    
                case Instructions::iDotChain: {
                    rc->stack->pop();
                    ++rc->pc;
                } break;

                case Instructions::iGetSharedVar: {
                    int index = popInt(rc->stack);
                    rc->stack->push(mce->sharedVars->get(index).lock());
                    ++rc->pc;
                } break;

                case Instructions::iGetGlobalVar: {
                    int index = LStackTopStringIndex(rc->stack);
                    rc->stack->pop();
                    const char* varName = &mce->exemplar->stringData[mce->exemplar->stringArray[index]];
                    std::vector<std::string> parts = Split(varName, '.', false, false);

                    std::shared_ptr<VarObj> vo = rc->engine->Global(parts[0].c_str()).lock();
                    if (!vo)
                        RaiseError(rc->pc, "Can't find global variable", parts[0].c_str());

                    if (parts.size() > 1) {
                        vo = vo->GetVar(parts[1].c_str()).lock();
                        if (!vo)
                            RaiseError(rc->pc, "Can't find global variable", varName);
                    }

                    rc->stack->push(vo);
                    ++rc->pc;
                } break;

                case Instructions::iGetSelfVar: {
					int varIndex = LStackTopStringIndex(rc->stack);
                    rc->stack->pop();
                    std::shared_ptr<VarObj> lvo = GetVar(varIndex).lock();
                    if (!lvo) {
                        if (varIndex > mce->exemplar->varCount)
                            RaiseError(rc->pc, "Unknown variable", 0);
                        else {
                            int varNameIndex = mce->exemplar->varNameIndex[varIndex];
                            RaiseError(rc->pc, "Can't find self var", &mce->exemplar->stringData[mce->exemplar->stringArray[varNameIndex]]);
                        }
                    }

                    rc->stack->push(lvo);
					++rc->pc; }
                    break;

                case Instructions::iGetLocalVarObj: {
					int paramIndex = instruction >> 16;
                    rc->stack->push(locals->get(paramIndex).lock());
                    ++rc->pc; }
                    break;

                case Instructions::iGetVarFromVar: {
                    shared_ptr<VarObj> vo = LStackTopVarObj(rc->stack).lock();
                    int func = instruction >> 16;
                    shared_ptr<VarObj> varVo = vo->GetVar(mce->exemplar->varNameIndex[func]).lock();
                    rc->stack->push(varVo);
                    ++rc->pc; }
                    break;

                default:
                    ++rc->pc;
                    break;
			}
            
            if (verboseTrace)
                stackPrint(rc->stack);
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



