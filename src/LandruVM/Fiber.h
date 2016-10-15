
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

namespace Landru
{
    struct FnEntry
	{
		char* name;
		VarObj::FuncPtr fn;
	};

    struct FnRuntime
    {
        std::weak_ptr<VarObj> ref;
        VarObj::FuncPtr fn;
    };
	
    class Engine;
    class Continuation;

	// Note: The Fiber is the execution context. So even if we are calling
	// functions on another machine, the fiber context is the caller not the
	// callee. The Fiber has the PC, stack, and variables. This distinction is
    // important when a machine is executing $ function calls.

    // Memory model
    //
    // class instance variables - currently called SelfVar, in variable _vars
    // class shared variables - currently called _sharedVars
    // named parameters - currently called rc->local
    // stack values - called rc->stack
    //


    class Fiber : public VarObj {
	public:

        Fiber(const char* name)
        : VarObj(name, &functions)
        , mce(0) {}
		
		virtual ~Fiber();

        VAROBJ_FACTORY(fiber, Fiber)

        enum RunEndCondition {
            kStateEnd,
            kSubStateEnd,
        };
		
		RunEndCondition Run(RunContext*);

        void Create(VarObjFactory*, int maxStackDepth, int maxVars, std::shared_ptr<MachineCacheEntry> mce_);

        const char* ExemplarName() const { return mce ? mce->exemplar->nameStr : 0; }
        int EntryPoint() const { return mce ? mce->exemplar->stateTable[mce->exemplar->stateIndex("main")] : -1; }

		virtual std::weak_ptr<VarObj> GetVar(int index) {
			if (_vars)
                return _vars->get(index);
            return std::weak_ptr<VarObj>();
		}

        virtual std::weak_ptr<VarObj> GetSharedVar(int index) {
            if (_sharedVars)
                return _sharedVars->get(index);
            return std::weak_ptr<VarObj>();
        }

	private:
        /*
        static int findFunction(FnRuntime* result,
                                std::shared_ptr<Fiber> self,
                                Engine* engine, const char* funcName);

        static int findFunctionS(VarObj* self,
                                Engine* engine, const char* funcName, 
                                VarObj** libObj);

        // recursive helper function where the first argument is the fiber to run
        static int findFunctionR(VarObj* self,
                                 Engine* engine,
                                 std::vector<std::string>& parts,
                                 int partIndex,
                                 VarObj** libObj);

        // recursive helper function where the fiber can't change, just recursing down vars
        static int findFunctionRV(VarObj* self,
                                  Engine* engine,
                                  std::vector<std::string>& parts, int partIndex,
                                  VarObj** libObj);
*/        
        std::shared_ptr<MachineCacheEntry> mce;   ///< Has exemplar and sharedVars

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
