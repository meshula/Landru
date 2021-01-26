
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRU_CONTINUATION_H
#define LANDRU_CONTINUATION_H

#include <vector>
#include "LandruVM/Fiber.h"
#include "LandruVM/VarObj.h"


// add children to the Continuation so that all children 
// can be cancelled when "this" is canceled.
// add ContinuationList to the Continuation so that the children 
//can be cancelled on the right engine.


namespace Landru
{
    class Engine;
    class ContinuationList;
    
    class Continuation
    {
    protected:
        std::shared_ptr<Fiber> _f;
        std::shared_ptr<VarObj> _data;
        Continuation*       _parent;
        int                 _progCounter;
        int                 _recurrances;
        float               _expiry;
        float               _recurranceDt;
        ContinuationList*   _continuationList;
        
        std::vector<Continuation*> _children;

        template <class T> friend class ContinuationT;
        template <class T> friend class ContinuationListT;
        friend class ContinuationList;

    public:
        Continuation(Continuation* parent, ContinuationList* l)
        : _f(0)
        , _parent(parent)
        , _progCounter(0)
        , _recurrances(0)
        , _expiry(0)
        , _recurranceDt(0)
        , _continuationList(l)
        {
        }

        virtual ~Continuation()
        {
            deallocate();
        }

    public:

        std::shared_ptr<Fiber> fiber() const { return _f; }
        void data(std::shared_ptr<VarObj> d) { _data = d; }
        std::weak_ptr<VarObj> data() const { return _data; }
        int pc() const { return _progCounter; }
        float expiry() const { return _expiry; }

        void allocate(std::shared_ptr<Fiber> f, LStack* s, int pc)
        {
            deallocate();
            _f = f;
            _progCounter = pc;
        }

        void deallocate();
    };

    class ContinuationList
    {
    protected:
        std::string _name;
        int _maxItems;
        bool _ordered;
        
        std::vector<Continuation*> _continuations;

    public:
        ContinuationList(const char* cname, int maxItems, bool ordered)
        : _name(cname)
        , _maxItems(maxItems)
        , _ordered(ordered)
        {
            _continuations.reserve(maxItems);
        }

        virtual ~ContinuationList()
        {
            expireAll();
        }
        
        const char* name() const { return _name.c_str(); }

        virtual void update(Engine*, float /*dt*/)
        {
        }
        
        void expireAll()
        {
            for (auto i = _continuations.begin(); i != _continuations.end(); ++i) {
                (*i)->deallocate();
            }
            _continuations.clear();
        }
        
        template<class FunctorCond>
        void expireConditional(FunctorCond& f)
        {
            auto i = _continuations.begin();
            while (i != _continuations.end()) {
                Continuation* a = *i;
                if (f.expire(a)) {
                    std::shared_ptr<Fiber> f = a->_f;
                    if (!f) {
                        /// @TODO should continuations be weak pointers?
                        // don't re-enqueue if there are only weak references remaining
                        a->deallocate();
                        i = _continuations.erase(i);
                        continue;
                    }
                    
                    float expiry = a->_expiry;
                    int pc = a->_progCounter;
                    int recurrances = a->_recurrances;
                    float recurranceDt = a->_recurranceDt;
                    Continuation* parent = a->_parent;
                    
                    a->deallocate();
                    i = _continuations.erase(i);
                    if (recurrances == -1) {
                        expiry += recurranceDt;
                        registerOrderedContinuation(parent, f, 0, expiry, pc, recurrances, recurranceDt);
                        i = _continuations.begin();
                        continue;
                    }
                    else if (recurrances > 0) {
                        expiry += recurranceDt;
                        registerOrderedContinuation(parent, f, 0, expiry, pc, recurrances - 1, recurranceDt);
                        i = _continuations.begin();
                        continue;
                    }
                }
                else
                    ++i;
            }
        }

/*
        // assuming an ordered list, expire all items whose data is less than or equal to expiry
        void orderedExpiry(T elapsed)
        {
            if (!ordered)
                RaiseError("ordered function on non-ordered items", 0);
            
            // throw away expired items
            auto i = _continuations.begin();
            while (i != _continuations.end()) {
                ContinuationT<T>* a = (ContinuationT<T>*) (*i);
                if (elapsed >= a->data()) {
                }
                else
                    ++i;
            }
        }
*/
        
        Continuation* registerContinuation(Continuation* parent,
                                           std::shared_ptr<Fiber> f,
                                           LStack* s,
                                           std::shared_ptr<VarObj> data,
                                           int pc)
        {
            Continuation* retval = new Continuation(parent, this);
            retval->allocate(f, s, pc);
            retval->data(data);
            if (parent)
                parent->_children.push_back(retval);
            _continuations.push_back(retval);
            return retval;
        }
        
        Continuation* registerOrderedContinuation(Continuation* parent,
                                                  std::shared_ptr<Fiber> f, LStack* s,
                                                  float expiry, int pc, int recurrances,
                                                  float dt)
        {
            Continuation* retval = new Continuation(parent, this);
            retval->allocate(f, s, pc);
            retval->_expiry = expiry;
            retval->_recurrances = recurrances;
            retval->_recurranceDt = dt;
            if (parent)
                parent->_children.push_back(retval);
            _continuations.push_back(retval);       // TODO, sort and optimize expiry
            return retval;
        }
        
        // clear all continuations referencing vop
        void clearContinuation(std::shared_ptr<Fiber> vop)
        {
            auto i = _continuations.begin();
            while (i != _continuations.end()) 
            {
                if ((*i)->fiber() == vop) 
                {
                    (*i)->deallocate();
                    i = _continuations.erase(i);
                }
                else
                    ++i;
            }
        }

        void clearContinuation(Continuation* c)
        {
            auto i = _continuations.begin();
            while (i != _continuations.end()) 
            {
                if ((*i) == c) 
                {
                    (*i)->deallocate();
                    i = _continuations.erase(i);
                }
                else
                    ++i;
            }
        }
        
        template<class FunctorData>
        void foreach(FunctorData& fd)
        {
            for (auto i = _continuations.begin(); i != _continuations.end(); ++i) 
            {
                if (!fd.apply(*i))
                    break;
            }
        }
        
    };

    unsigned int      Queue(const char* type, const char* eventName);
    ContinuationList* Queue(unsigned int index);
    void              QueueDeregister(ContinuationList*);
    void              QueueRegister(ContinuationList*, const char* type, const char* eventName);
    void              UpdateQueues(Engine*, float elapsedTime);
    void              ClearQueues(std::shared_ptr<Fiber> fiber);
    
} // Landru

#endif


// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruVM/Continuation.h"

#include <map>
#include <string>

namespace Landru
{
    
struct QueueRecord
{
    std::string  name;
    ContinuationList* queue;
};

std::vector<QueueRecord>& queues()
{
    static std::vector<QueueRecord>* _queues = new std::vector<QueueRecord>();
    return *_queues;        
}

unsigned int Queue(const char* type, const char* eventName)
{
    char* temp = (char*) alloca(strlen(type) + strlen(eventName) + 1);
    strcpy(temp, type);
    strcat(temp, eventName);

    int index = 0;
    std::vector<QueueRecord>& q = queues();
    for (std::vector<QueueRecord>::const_iterator i = q.begin(); i != q.end(); ++i, ++index) 
    {
        if (!strcmp((*i).name.c_str(), temp))
            return index;
    }
    
    return -1;
}
        
ContinuationList* Queue(unsigned int index)
{
    return queues()[index].queue;
}

void QueueRegister(ContinuationList* oib, const char* type, const char* eventName)
{
    char* temp = (char*) alloca(strlen(type) + strlen(eventName) + 1);
    strcpy(temp, type);
    strcat(temp, eventName);
    std::vector<QueueRecord>& q = queues();
    q.push_back(QueueRecord());
    q.back().name.assign(temp);
    q.back().queue = oib;
}
    
void QueueDeregister(ContinuationList* cl)
{
    // QueueRegister allows duplicate registrations, this removes every instance of cl.
    std::vector<QueueRecord>& q = queues();
    std::vector<QueueRecord>::iterator i = q.begin();
    while (i != q.end()) 
    {
        if ((*i).queue == cl) 
        {
            q.erase(i);
            i = q.begin();
        }
        else
            ++i;
    }
}

void UpdateQueues(Engine* engine, float elapsedTime)
{
    std::vector<QueueRecord>& q = queues();
    for (auto i : q)
        i.queue->update(engine, elapsedTime);
}

void ClearQueues(std::shared_ptr<Fiber> f)
{
    std::vector<QueueRecord>& q = queues();
    for (auto i : q)
        i.queue->clearContinuation(f);
}
    
    
void Continuation::deallocate()
{
    for (auto i = _children.begin(); i != _children.end(); ++i)
        (*i)->_continuationList->clearContinuation(*i);
    
    if (_f)
        _f.reset();
}
    
    
} // Landru


// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include "LandruVM/Continuation.h"

namespace Landru {  
    class Engine;
    
    class TimeVarObj : public VarObj
    {       
    public:
        TimeVarObj(const char* name);
        virtual ~TimeVarObj();
        
        VAROBJ_FACTORY(time, TimeVarObj);        
        
        LANDRU_DECL_BINDING_BEGIN
            LANDRU_DECL_BINDING(after)
            LANDRU_DECL_BINDING(every)
            LANDRU_DECL_BINDING(recur)
        LANDRU_DECL_BINDING_END
    };
    
    class TimeEngine : public ContinuationList
    {
        struct TimeFunctor;
        
    public:
        TimeEngine();
        
        // this update is called from updateQueues in Continuation.cpp
        virtual void update(Engine* engine, float elapsedTime);
    };
    
}

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
            if (vop) 
            {
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

        LStack stack;
        for (size_t i = 0; i < checkForRunners.runners.size(); ++i) 
        {
            // refs were incremented above
            std::shared_ptr<Fiber> f = checkForRunners.runners[i];

            Landru::RunContext rc;
            rc.engine = engine;
            rc.fiber = checkForRunners.runners[i];
            rc.elapsedTime = elapsedTime;
            rc.pc = checkForRunners.pc[i];
            rc.stack = &stack;
            rc.continuationContext = checkForRunners.continuations[i];
            rc.locals = 0;
            checkForRunners.runners[i]->Run(&rc);
            stack.clear();
        }

        checkForRunners.runners.clear();
        checkForRunners.pc.clear();
    }
    

    TimeVarObj::TimeVarObj(const char* name)
    : VarObj(name, &functions) {}
    
    TimeVarObj::~TimeVarObj() {}
     
    LANDRU_DECL_FN(TimeVarObj, after) 
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();
        float dt = voa->getReal(-1);
        if (dt != FLT_MAX) 
        {
            int recurrances = 0;
            p->engine->timeEngine()->registerOrderedContinuation(p->contextContinuation,
                                                                 p->fiber, p->stack,
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
                                                                 p->fiber, p->stack,
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
                                                                 p->fiber, p->stack,
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



