
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include "LandruVM/Continuation.h"
#include "LandruVM/VarObj.h"
#include "LandruStd/ConcurrentQueue.h"

namespace Landru {

    class MouseEvent
    {
    public:
        enum Kind { kUp, kDown, kMove, kDrag };

        MouseEvent(int x_, int y_, Kind k_) : x(x_), y(y_), kind(k_) { }
        int x, y;
        Kind kind;
    };
    
    class MouseEngine : public ContinuationList
    {
        struct MouseFunctor;
        
    public:
        MouseEngine();
        virtual ~MouseEngine();
        
        // this update is called from updateQueues in Continuation.cpp
        virtual void update(::Landru::Engine* engine, float elapsedTime);
        
        void enqueue(int x, int y, MouseEvent::Kind);
        
        std::weak_ptr<IntVarObj> up();
        std::weak_ptr<IntVarObj> down();
        std::weak_ptr<IntVarObj> drag();
        std::weak_ptr<IntVarObj> move();
        
    private:
        concurrent_queue<MouseEvent*> events;
        std::shared_ptr<IntVarObj> vup;
        std::shared_ptr<IntVarObj> vdown;
        std::shared_ptr<IntVarObj> vdrag;
        std::shared_ptr<IntVarObj> vmove;
    };
    
    class MouseVarObj : public Landru::VarObj
    {
    public:
        MouseVarObj(const char* name);
        virtual ~MouseVarObj();
        
        //virtual void Update(float elapsedTime)
        //{
        //}
        
        VAROBJ_FACTORY(mouse, MouseVarObj)
        
        LANDRU_DECL_BINDING_BEGIN
        LANDRU_DECL_BINDING(up)
        LANDRU_DECL_BINDING(down)
        LANDRU_DECL_BINDING(drag)
        LANDRU_DECL_BINDING(move)
        LANDRU_DECL_BINDING_END
        
    private:
        MouseEngine* mouseEngine;
    };
    
	class IoVarObj : public Landru::VarObj
    {
    public:
        IoVarObj(const char* name);
        virtual ~IoVarObj() { }

        //virtual void Update(float elapsedTime)
		//{
		//}

        static void setPrintFn(void (*fn)(const char*));
        
        VAROBJ_FACTORY(io, IoVarObj)

        LANDRU_DECL_BINDING_BEGIN
            LANDRU_DECL_BINDING(print)
        LANDRU_DECL_BINDING_END
    };


}
