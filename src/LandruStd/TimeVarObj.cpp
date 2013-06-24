
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruStd/TimeVarObj.h"

#include "LandruVM/Engine.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/Stack.h"
#include "LandruVM/VarObjStackUtil.h"

#include <float.h>

namespace Landru {
    
    struct TimeEngine::TimeFunctor
    {
        VarPool* v;
        std::vector<VarObjPtr*> runners;
        std::vector<int> pc;
        std::vector<Continuation*> continuations;
        
        TimeFunctor(VarPool* v) 
        : v(v)
        , elapsedTime(0) { }
        
        // all items that have expired are pushed onto the runners pile
        bool apply(Continuation* curr)
        {
            if (elapsedTime < curr->expiry())
                return true; // keep processing the list

            VarObjPtr* vop = curr->fiber();
            if (vop && vop->vo) {
                v->addStrongRef(vop);
                runners.push_back(vop);
                pc.push_back(curr->pc());
                continuations.push_back(curr);
            }
            return true; // keep processing the list
        }
        
        float elapsedTime;
    };
    
    struct FunctorCond
    {
        bool expire(Continuation* c)
        {
            return t >= c->expiry();
        }
        
        float t;
    };
    
    TimeEngine::TimeEngine(VarPool* v)
    : ContinuationList("time", v, 1024, true) // true because ordered
    {
    }
        
    void TimeEngine::update(Engine* engine, float elapsedTime)
    {
        TimeFunctor checkForRunners(_varPool);
        checkForRunners.elapsedTime = elapsedTime;
        foreach(checkForRunners);

        FunctorCond fc;
        fc.t = elapsedTime;
        expireConditional(fc);
        
        for (size_t i = 0; i < checkForRunners.runners.size(); ++i) {
            // refs were incremented above
            LStack* stack = LStackGetFromPool(1024);
            Fiber* f = (Fiber*) checkForRunners.runners[i]->vo;
            VarObjArray exeStack("timeEngineExeStack");
            f->Run(engine,
                   checkForRunners.runners[i],
                   elapsedTime,
                   checkForRunners.pc[i],
                   stack,
                   checkForRunners.continuations[i],
                   &exeStack, 0);
            LStackReleaseToPool(stack, (LVarPool*) _varPool);
        }
        for (size_t i = 0; i < checkForRunners.runners.size(); ++i)
            _varPool->releaseStrongRef(checkForRunners.runners[i]);

        checkForRunners.runners.clear();
        checkForRunners.pc.clear();
    }
    

    TimeVarObj::TimeVarObj(const char* name)
    : VarObj(name, &functions)
    {
    }
    
    TimeVarObj::~TimeVarObj()
    {
    }
     
    LANDRU_DECL_FN(TimeVarObj, after)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

        float dt = voa->getReal(-1);
        if (dt != FLT_MAX)
        {
            int recurrances = 0;
            p->engine->timeEngine()->registerOrderedContinuation(p->parentContinuation,
                                                                 p->self, p->stack,
                                                                 p->engine->elapsedTime() + dt,
                                                                 p->continuationPC,
                                                                 recurrances, dt);
        }
	}

    LANDRU_DECL_FN(TimeVarObj, every)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

        float dt = voa->getReal(-1);

        if (dt != FLT_MAX)
        {
            int recurrances = -1;
            p->engine->timeEngine()->registerOrderedContinuation(p->parentContinuation,
                                                                 p->self, p->stack,
                                                                 p->engine->elapsedTime() + dt,
                                                                 p->continuationPC, recurrances, dt);
        }
	}
    
    LANDRU_DECL_FN(TimeVarObj, recur)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

        float dt = voa->getReal(-2);
        int recurrances = voa->getInt(-1);

        if (dt != FLT_MAX)
        {
            p->engine->timeEngine()->registerOrderedContinuation(p->parentContinuation,
                                                                 p->self, p->stack,
                                                                 p->engine->elapsedTime() + dt,
                                                                 p->continuationPC, recurrances, dt);
        }
	}
    
    LANDRU_DECL_TABLE_BEGIN(TimeVarObj)
        LANDRU_DECL_ENTRY(TimeVarObj, after)
        LANDRU_DECL_ENTRY(TimeVarObj, every)
        LANDRU_DECL_ENTRY(TimeVarObj, recur)
    LANDRU_DECL_TABLE_END(TimeVarObj)
    
} // Landru

