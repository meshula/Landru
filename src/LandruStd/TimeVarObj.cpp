
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
        std::vector<std::shared_ptr<Fiber>> runners;
        std::vector<int> pc;
        std::vector<Continuation*> continuations;
        
        TimeFunctor()
        : elapsedTime(0) { }
        
        // all items that have expired are pushed onto the runners pile
        bool apply(Continuation* curr)
        {
            if (elapsedTime < curr->expiry())
                return true; // keep processing the list

            std::shared_ptr<Fiber> vop = curr->fiber();
            if (vop) {
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
    
    TimeEngine::TimeEngine()
    : ContinuationList("time", 1024, true) // true because ordered
    {
    }
        
    void TimeEngine::update(Engine* engine, float elapsedTime)
    {
        TimeFunctor checkForRunners;
        checkForRunners.elapsedTime = elapsedTime;
        foreach(checkForRunners);

        FunctorCond fc;
        fc.t = elapsedTime;
        expireConditional(fc);
        
        std::shared_ptr<LStack> stack = std::make_shared<LStack>();
        for (size_t i = 0; i < checkForRunners.runners.size(); ++i) {
            // refs were incremented above
            std::shared_ptr<Fiber> f = checkForRunners.runners[i];

            Landru::RunContext rc;
            rc.engine = engine;
            rc.self = checkForRunners.runners[i];
            rc.elapsedTime = elapsedTime;
            rc.pc = checkForRunners.pc[i];
            rc.stack = stack.get();
            rc.continuationContext = checkForRunners.continuations[i];
            rc.locals = 0;
            f->Run(&rc);
            stack->clear();
        }

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
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();
        float dt = voa->getReal(-1);
        if (dt != FLT_MAX)
        {
            int recurrances = 0;
            p->engine->timeEngine()->registerOrderedContinuation(p->contextContinuation,
                                                                 p->f, p->stack,
                                                                 p->engine->elapsedTime() + dt,
                                                                 p->continuationPC,
                                                                 recurrances, dt);
        }
	}

    LANDRU_DECL_FN(TimeVarObj, every)
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();
        float dt = voa->getReal(-1);

        if (dt != FLT_MAX)
        {
            int recurrances = -1;
            p->engine->timeEngine()->registerOrderedContinuation(p->contextContinuation,
                                                                 p->f, p->stack,
                                                                 p->engine->elapsedTime() + dt,
                                                                 p->continuationPC, recurrances, dt);
        }
	}
    
    LANDRU_DECL_FN(TimeVarObj, recur)
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();
        float dt = voa->getReal(-2);
        int recurrances = voa->getInt(-1);

        if (dt != FLT_MAX)
        {
            p->engine->timeEngine()->registerOrderedContinuation(p->contextContinuation,
                                                                 p->f, p->stack,
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

