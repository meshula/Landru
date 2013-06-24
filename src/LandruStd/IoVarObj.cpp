
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruStd/IoVarObj.h"

#include "LandruStd/IntVarObj.h"
#include "LandruStd/RealVarObj.h"
#include "LandruStd/StringVarObj.h"
#include "LandruVM/VarObjArray.h"
#include "LandruVM/Engine.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/Stack.h"
#include "LandruVM/VarObjStackUtil.h"

#include <string.h>
#include <stdio.h>

namespace Landru {
    
    void (*printFn)(const char*) =  0;
    void IoVarObj::setPrintFn(void (*fn)(const char*))
    {
        printFn = fn;
    }

    IoVarObj::IoVarObj(const char* name)
    : VarObj(name, &functions)
    {
    }

    LANDRU_DECL_FN(IoVarObj, print)
	{
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);
        
        int s = voa->size();
        for (int i = 0; i < s; ++i) {
            VarObjPtr* vop = voa->get(i);
            VarObj* vo = vop->vo;
            
            if (vo->TypeId() == StringVarObj::StaticTypeId()) {
                StringVarObj* svo = (StringVarObj*) vo;
                
                if (svo) {
                    const char* printString = svo->getCstr();
                    if (printFn)
                        printFn(printString);
                    else
                        printf("%s\n", printString);
                }
            }
            else if (vo->TypeId() == IntVarObj::StaticTypeId()) {
                IntVarObj* ivo = (IntVarObj*) vo;
                printf("%d\n", ivo->value());
            }
            else if (vo->TypeId() == RealVarObj::StaticTypeId()) {
                RealVarObj* ivo = (RealVarObj*) vo;
                printf("%f\n", ivo->value());
            }
        }
	}

    LANDRU_DECL_TABLE_BEGIN(IoVarObj)
        LANDRU_DECL_ENTRY(IoVarObj, print)
    LANDRU_DECL_TABLE_END(IoVarObj)


    struct MouseEngine::MouseFunctor
    {
        VarPool* v;
        std::vector<VarObjPtr*> runners;
        std::vector<int> pc;
        std::vector<Continuation*> continuations;
        std::vector<std::pair<int, int> > coords;
        MouseEvent* event;
        MouseEngine* engine;
        
        MouseFunctor(VarPool* v, MouseEngine* en, MouseEvent* e)
        : v(v)
        , event(e)
        , engine(en)
        { }
        
        // all items that have expired are pushed onto the runners pile
        bool apply(Continuation* curr)
        {
            VarObjPtr* cd = curr->data();
            if ((event->kind == MouseEvent::kUp   && cd == engine->up())     ||
                (event->kind == MouseEvent::kDown && cd == engine->down()) ||
                (event->kind == MouseEvent::kDrag && cd == engine->drag()) ||
                (event->kind == MouseEvent::kMove && cd == engine->move()))
            {
                VarObjPtr* vop = curr->fiber();
                if (vop && vop->vo) {
                    v->addStrongRef(vop);
                    runners.push_back(vop);
                    pc.push_back(curr->pc());
                    continuations.push_back(curr);
                    coords.push_back(std::pair<int, int>(event->x, event->y));
                }
            }
            return true; // keep processing the list
        }
        
    };
    
    MouseEngine::MouseEngine(VarPool* v)
    : ContinuationList("mouse", v, 1024, false) // true because ordered
    , vup(0), vdown(0), vdrag(0), vmove(0)
    {
    }
    
    MouseEngine::~MouseEngine()
    {
        _varPool->releaseStrongRef(vup);
        _varPool->releaseStrongRef(vdown);
        _varPool->releaseStrongRef(vdrag);
        _varPool->releaseStrongRef(vmove);
    }
    
    void MouseEngine::enqueue(int x, int y, MouseEvent::Kind k)
    {
        events.push(new MouseEvent(x, y, k));
    }

    void MouseEngine::update(::Landru::Engine* engine, float elapsedTime)
    {
        MouseEvent* event;
        while (events.try_pop(event)) {
            MouseFunctor functor(_varPool, this, event);
            foreach(functor);
        
            for (size_t i = 0; i < functor.runners.size(); ++i) {
                // refs were incremented above
                LStack* stack = LStackGetFromPool(1024);
                Fiber* f = (Fiber*) functor.runners[i]->vo;
                std::pair<int, int>& xy = functor.coords[i];

                Landru::VarObjArray exeStack("mouseEngineExeStack");
                exeStack.push_back(functor.runners[i]);
                
                Landru::VarObjArray locals("locals");
                
                RealVarObj* vo = new RealVarObj("real");
                vo->set(xy.first);
                VarObjPtr* vop = engine->varPool()->allocVarObjSlot(0, vo, true);
                locals.add(vop);
                vo = new RealVarObj("real");
                vo->set(xy.second);
                vop = engine->varPool()->allocVarObjSlot(0, vo, true);
                locals.add(vop);
                
                f->Run(engine, functor.runners[i], elapsedTime, functor.pc[i], stack, functor.continuations[i], &exeStack, &locals);
                LStackReleaseToPool(stack, (LVarPool*) _varPool);
            }
            for (size_t i = 0; i < functor.runners.size(); ++i)
                _varPool->releaseStrongRef(functor.runners[i]);
            
            delete event;
        }
    }
    
    VarObjPtr* MouseEngine::up()
    {
        if (!vup) {
            vup = IntVarObj::Factory(_varPool, &vup, "__mouseUpVal");
            _varPool->addStrongRef(vup);
        }
        return vup;
    }
    
    VarObjPtr* MouseEngine::down()
    {
        if (!vdown) {
            vdown = IntVarObj::Factory(_varPool, &vdown, "__mouseDownVal");
            _varPool->addStrongRef(vdown);
        }
        return vdown;
    }

    VarObjPtr* MouseEngine::drag()
    {
        if (!vdrag) {
            vdrag = IntVarObj::Factory(_varPool, &vdrag, "__mouseDragVal");
            _varPool->addStrongRef(vdrag);
        }
        return vdrag;
    }

    VarObjPtr* MouseEngine::move()
    {
        if (!vmove) {
            vmove = IntVarObj::Factory(_varPool, &vmove, "__mouseMoveVal");
            _varPool->addStrongRef(vmove);
        }
        return vmove;
    }

    MouseVarObj::MouseVarObj(const char* name)
    : VarObj(name, &functions)
    {
    }
    
    MouseVarObj::~MouseVarObj()
    {
    }

    VarObjPtr* upVop = 0;
    VarObjPtr* downVop = 0;
    VarObjPtr* dragVop = 0;
    VarObjPtr* moveVop = 0;

    // on lib mouse.up --- there is nothing on the stack
    LANDRU_DECL_FN(MouseVarObj, up)
    {
        MouseEngine* me = p->engine->mouseEngine();
        me->registerContinuation(p->parentContinuation,
                                 p->self,   // the Fiber to run on
                                 p->stack,
                                 me->up(),
                                 p->continuationPC);    // adds strong ref to self
    }
    
    // on lib mouse.down --- there is nothing on the stack
    LANDRU_DECL_FN(MouseVarObj, down)
    {
        MouseEngine* me = p->engine->mouseEngine();
        me->registerContinuation(p->parentContinuation,
                                 p->self,   // the Fiber to run on
                                 p->stack,
                                 me->down(),
                                 p->continuationPC);    // adds strong ref to self
    }
    
    // on lib mouse.drag --- there is nothing on the stack
    LANDRU_DECL_FN(MouseVarObj, drag)
    {
        MouseEngine* me = p->engine->mouseEngine();
        me->registerContinuation(p->parentContinuation,
                                 p->self,   // the Fiber to run on
                                 p->stack,
                                 me->drag(),
                                 p->continuationPC);    // adds strong ref to self
    }

    // on lib mouse.move --- there is nothing on the stack
    LANDRU_DECL_FN(MouseVarObj, move)
    {
        MouseEngine* me = p->engine->mouseEngine();
        me->registerContinuation(p->parentContinuation,
                                 p->self,   // the Fiber to run on
                                 p->stack,
                                 me->move(),
                                 p->continuationPC);    // adds strong ref to self
    }

    LANDRU_DECL_TABLE_BEGIN(MouseVarObj)
        LANDRU_DECL_ENTRY(MouseVarObj, up)
        LANDRU_DECL_ENTRY(MouseVarObj, down)
        LANDRU_DECL_ENTRY(MouseVarObj, drag)
        LANDRU_DECL_ENTRY(MouseVarObj, move)
    LANDRU_DECL_TABLE_END(MouseVarObj)
}
