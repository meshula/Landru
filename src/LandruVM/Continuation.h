
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRU_CONTINUATION_H
#define LANDRU_CONTINUATION_H

#include <vector>
#include "LandruVM/Fiber.h"
#include "LandruVM/VarObj.h"


//add children to the Continuation so that all children can be cancelled when "this" is canceled.
//add ContinuationList to the Continuation so that the children can be cancelled on the right engine.


namespace Landru
{
    class Engine;
    class ContinuationList;
    
    class Continuation
    {
    public:
        Continuation(VarPool* v, Continuation* parent, ContinuationList* l)
        : _f(0)
        , _varPool(v)
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

        VarObjPtr* fiber() const { return _f; }
        void data(VarObjPtr* d) { _data = d; }
        VarObjPtr* data() const { return _data; }
        int pc() const { return _progCounter; }
        VarPool* varPool() const { return _varPool; }
        float expiry() const { return _expiry; }

        void allocate(VarObjPtr* f, LStack* s, int pc)
        {
            deallocate();
            _varPool->addStrongRef(f);
            
            _f = f;
            _progCounter = pc;
        }

        void deallocate();
        
    protected:
        VarObjPtr*          _f;
        VarObjPtr*          _data;
        VarPool*            _varPool;
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
    };

    class ContinuationList
    {
    public:
        ContinuationList(const char* cname, VarPool* v, int maxItems, bool ordered)
        : _name(cname)
        , _varPool(v)
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
                    VarObjPtr* f = a->_f;
                    if (!f->vo) {
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
                                           VarObjPtr* f, LStack* s,
                                           VarObjPtr* data,
                                           int pc)
        {
            Continuation* retval = new Continuation(_varPool, parent, this);
            retval->allocate(f, s, pc);
            retval->data(data);
            if (parent)
                parent->_children.push_back(retval);
            _continuations.push_back(retval);
            _varPool->addStrongRef(f);
            return retval;
        }
        
        Continuation* registerOrderedContinuation(Continuation* parent,
                                                  VarObjPtr* f, LStack* s,
                                                  float expiry, int pc, int recurrances,
                                                  float dt)
        {
            Continuation* retval = new Continuation(_varPool, parent, this);
            retval->allocate(f, s, pc);
            retval->_expiry = expiry;
            retval->_recurrances = recurrances;
            retval->_recurranceDt = dt;
            if (parent)
                parent->_children.push_back(retval);
            _continuations.push_back(retval);       // TODO, sort and optimize expiry
            _varPool->addStrongRef(f);
            return retval;
        }
        
        // clear all continuations referencing vop
	    void clearContinuation(VarObjPtr* vop)
	    {
            auto i = _continuations.begin();
            while (i != _continuations.end()) {
                if ((*i)->fiber() == vop) {
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
            while (i != _continuations.end()) {
                if ((*i) == c) {
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
            for (auto i = _continuations.begin(); i != _continuations.end(); ++i) {
                if (!fd.apply(*i))
                    break;
            }
        }
        
    protected:
        std::string _name;
        int _maxItems;
        VarPool* _varPool;
        bool _ordered;
        
        std::vector<Continuation*> _continuations;
    };

	unsigned int      Queue(const char* type, const char* eventName);
    ContinuationList* Queue(unsigned int index);
    void              QueueDeregister(ContinuationList*);
	void		      QueueRegister(ContinuationList*, const char* type, const char* eventName);
	void		      UpdateQueues(Engine*, float elapsedTime);
	void		      ClearQueues(VarObjPtr* fiber);    
    
} // Landru

#endif
