
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRU_RUNTIME_FIBER_H
#define LANDRU_RUNTIME_FIBER_H

#ifdef __cplusplus

#include "LandruVM/Exemplar.h"
#include "LandruVM/MachineCache.h"
#include "LandruVM/VarObj.h"
#include "LandruVM/VarObjArray.h"

#include <string.h>
#include <vector>
#include <set>

typedef struct LStack LStack;

namespace Landru
{
    struct FnEntry
	{
		char* name;
		VarObj::FuncPtr fn;
	};

    struct FnRuntime
    {
        VarObjWeakRef* ref;
        VarObj::FuncPtr fn;
    };
	
	class VarPool;
    class Engine;
    class Continuation;

	// Note: The Fiber is the execution context. So even if we are calling
	// functions on another machine, the fiber context is the caller not the
	// callee. The Fiber has the PC, stack, and variables. This distinction is
    // important when a machine is executing $ function calls.

    class Fiber : public VarObj
	{
        // Fibers can only be created from the varpool, by the factory.
        // ctor/dtor private to prevent subclassing.
        
        Fiber(const char* name)
        : VarObj(name, &functions)
        , mce(0), _suspendedPC(0)
        {
        }
		
		virtual ~Fiber();

	public:
        VAROBJ_FACTORY(fiber, Fiber)

        enum RunEndCondition
        {
            kStateEnd,
            kSubStateEnd,
            kStateSuspend,
        };
		
		RunEndCondition Run(Engine*, VarObjPtr* self, float elapsedTime,
                            int pc, LStack*, Continuation* continuationContext, VarObjArray* exeStack, VarObjArray* locals);

        void Create(VarPool* varpool, int maxStackDepth, int maxVars, VarObjPtr* self, const MachineCacheEntry* mce_);

        const char*     ExemplarName() const { return mce->exemplar->nameStr; }
        int             EntryPoint() const { return mce->exemplar->stateTable[mce->exemplar->stateIndex("main")]; }

		virtual VarObjPtr* GetVar(int index)
		{
			if (index >= mce->exemplar->varCount)
                return _vars->varObj(mce, index - mce->exemplar->varCount);
			else
				return _vars->varObj(this, index);
		}
        
        virtual VarObjPtr* GetSharedVar(int index)
        {
            return _vars->varObj(mce, index);
        }

        static VarObjPtr* MainFiber() { return main; }
        static void SetMainFiber(VarObjPtr* f) { main = f; }
        static std::set<Fiber*>& ActiveFibers();
		
	private:
        static int findFunction(FnRuntime* result,
                                VarObjPtr* self, 
                                Engine* engine, const char* funcName);

        static int findFunctionS(VarObjPtr* self, 
                                Engine* engine, const char* funcName, 
                                VarObjPtr** libObj);

        // recursive helper function where the first argument is the fiber to run
        static int findFunctionR(VarObjPtr* self, 
                                 Engine* engine,
                                 std::vector<std::string>& parts,
                                 int partIndex,
                                 VarObjPtr** libObj);

        // recursive helper function where the fiber can't change, just recursing down vars
        static int findFunctionRV(VarObjPtr* self, 
                                  Engine* engine,
                                  std::vector<std::string>& parts, int partIndex,
                                  VarObjPtr** libObj);
        
        const MachineCacheEntry* mce;   ///< Has exemplar and sharedVars
        
        FnRuntime* resolvedFunctionArray;

		int _suspendedPC;

        static VarObjPtr*   main;           ///< There is only one main fiber
        
        LANDRU_DECL_BINDING_BEGIN
            LANDRU_DECL_BINDING(add)
            LANDRU_DECL_BINDING(sub)
            LANDRU_DECL_BINDING(mul)
            LANDRU_DECL_BINDING(div)
            LANDRU_DECL_BINDING(sqrt)

            LANDRU_DECL_BINDING(min)
            LANDRU_DECL_BINDING(max)

            LANDRU_DECL_BINDING(int)

            LANDRU_DECL_BINDING(toggle)

//          LANDRU_DECL_BINDING(send)
            LANDRU_DECL_BINDING(new)
        LANDRU_DECL_BINDING_END
    };

} // Landru

#endif // __cplusplus

typedef struct LFiber LFiber;

#endif
