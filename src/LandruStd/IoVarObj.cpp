
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
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

        int s = voa->size();
        for (int i = 0; i < s; ++i) {
            VarObj* vo = voa->get(i).lock().get();

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
        std::vector<std::shared_ptr<Fiber>> runners;
        std::vector<int> pc;
        std::vector<Continuation*> continuations;
        std::vector<std::pair<int, int> > coords;
        MouseEvent* event;
        MouseEngine* engine;
        
        MouseFunctor(MouseEngine* en, MouseEvent* e)
        : event(e)
        , engine(en)
        { }
        
        // all items that have expired are pushed onto the runners pile
        bool apply(Continuation* curr)
        {
            std::shared_ptr<VarObj> cd = curr->data().lock();
            if ((event->kind == MouseEvent::kUp   && cd == engine->up().lock())     ||
                (event->kind == MouseEvent::kDown && cd == engine->down().lock()) ||
                (event->kind == MouseEvent::kDrag && cd == engine->drag().lock()) ||
                (event->kind == MouseEvent::kMove && cd == engine->move().lock()))
            {
                std::shared_ptr<Fiber> vop = curr->fiber();
                if (vop) {
                    runners.push_back(vop);
                    pc.push_back(curr->pc());
                    continuations.push_back(curr);
                    coords.push_back(std::pair<int, int>(event->x, event->y));
                }
            }
            return true; // keep processing the list
        }
        
    };
    
    MouseEngine::MouseEngine()
    : ContinuationList("mouse", 1024, false) // true because ordered
    {
    }
    
    MouseEngine::~MouseEngine()
    {
    }
    
    void MouseEngine::enqueue(int x, int y, MouseEvent::Kind k)
    {
        events.push(new MouseEvent(x, y, k));
    }

    void MouseEngine::update(::Landru::Engine* engine, float elapsedTime)
    {
        MouseEvent* event;
        while (events.try_pop(event)) {
            MouseFunctor functor(this, event);
            foreach(functor);
        
            for (size_t i = 0; i < functor.runners.size(); ++i) {
                // refs were incremented above
                LStack* stack = LStackGetFromPool(1024);
                std::pair<int, int>& xy = functor.coords[i];

                Landru::VarObjArray locals("locals");
                
                RealVarObj* vo = new RealVarObj("real");
                vo->set(xy.first);
                locals.add(std::shared_ptr<VarObj>(vo));
                vo = new RealVarObj("real");
                vo->set(xy.second);
                locals.add(std::shared_ptr<VarObj>(vo));

                Landru::RunContext rc;
                rc.engine = engine;
                rc.fiber = functor.runners[i];
                rc.elapsedTime = elapsedTime;
                rc.pc = functor.pc[i];
                rc.stack = stack;
                rc.continuationContext = functor.continuations[i];
                rc.locals = &locals;
                functor.runners[i]->Run(&rc);
            }
            delete event;
        }
    }

    std::weak_ptr<IntVarObj> MouseEngine::up()
    {
        if (!vup) {
            vup = std::make_shared<IntVarObj>("__mouseUpVal");
        }
        return vup;
    }
    
    std::weak_ptr<IntVarObj> MouseEngine::down()
    {
        if (!vdown) {
            vdown = std::make_shared<IntVarObj>("__mouseDownVal");
        }
        return vdown;
    }

    std::weak_ptr<IntVarObj> MouseEngine::drag()
    {
        if (!vdrag) {
            vdrag = std::make_shared<IntVarObj>("__mouseDragVal");
        }
        return vdrag;
    }

    std::weak_ptr<IntVarObj> MouseEngine::move()
    {
        if (!vmove) {
            vmove = std::make_shared<IntVarObj>("__mouseMoveVal");
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

    // on lib mouse.up --- there is nothing on the stack
    LANDRU_DECL_FN(MouseVarObj, up)
    {
        p->stack->pop();
        MouseEngine* me = p->engine->mouseEngine();
        me->registerContinuation(p->contextContinuation,
                                 p->fiber,   // the Fiber to run on
                                 p->stack,
                                 me->up().lock(),
                                 p->continuationPC);    // adds strong ref to self
    }
    
    // on lib mouse.down --- there is nothing on the stack
    LANDRU_DECL_FN(MouseVarObj, down)
    {
        MouseEngine* me = p->engine->mouseEngine();
        p->stack->pop();
        me->registerContinuation(p->contextContinuation,
                                 p->fiber,   // the Fiber to run on
                                 p->stack,
                                 me->down().lock(),
                                 p->continuationPC);    // adds strong ref to self
    }
    
    // on lib mouse.drag --- there is nothing on the stack
    LANDRU_DECL_FN(MouseVarObj, drag)
    {
        MouseEngine* me = p->engine->mouseEngine();
        p->stack->pop();
        me->registerContinuation(p->contextContinuation,
                                 p->fiber,   // the Fiber to run on
                                 p->stack,
                                 me->drag().lock(),
                                 p->continuationPC);    // adds strong ref to self
    }

    // on lib mouse.move --- there is nothing on the stack
    LANDRU_DECL_FN(MouseVarObj, move)
    {
        MouseEngine* me = p->engine->mouseEngine();
        p->stack->pop();
        me->registerContinuation(p->contextContinuation,
                                 p->fiber,   // the Fiber to run on
                                 p->stack,
                                 me->move().lock(),
                                 p->continuationPC);    // adds strong ref to self
    }

    LANDRU_DECL_TABLE_BEGIN(MouseVarObj)
        LANDRU_DECL_ENTRY(MouseVarObj, up)
        LANDRU_DECL_ENTRY(MouseVarObj, down)
        LANDRU_DECL_ENTRY(MouseVarObj, drag)
        LANDRU_DECL_ENTRY(MouseVarObj, move)
    LANDRU_DECL_TABLE_END(MouseVarObj)
}
