
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include "LandruVM/VarObj.h"
#include "LandruVM/Engine.h"
#include "LandruVM/Stack.h"
#include "LandruStd/IntVarObj.h"
#include "LandruStd/RealVarObj.h"
#include "LandruStd/StringVarObj.h"

namespace Landru {

    inline
    void pushInt(FnParams* p, int v)
    {
        std::shared_ptr<VarObj> i = std::make_shared<IntVarObj>("int");
        IntVarObj* ivo = (IntVarObj*) i.get();
        ivo->set(v);
        LStackPushVarObj(p->stack, i);
    }
    
    inline
    void pushInt(LStack* s, int v)
    {
        std::shared_ptr<VarObj> i = std::make_shared<IntVarObj>("int");
        IntVarObj* ivo = (IntVarObj*) i.get();
        ivo->set(v);
        LStackPushVarObj(s, i);
    }

    inline
    void pushReal(FnParams* p, float v)
    {
        std::shared_ptr<VarObj> i = std::make_shared<RealVarObj>("int");
        RealVarObj* ivo = (RealVarObj*) i.get();
        ivo->set(v);
        LStackPushVarObj(p->stack, i);
    }
    
    inline
    void pushReal(LStack* s, float v)
    {
        std::shared_ptr<VarObj> i = std::make_shared<RealVarObj>("int");
        RealVarObj* ivo = (RealVarObj*) i.get();
        ivo->set(v);
        LStackPushVarObj(s, i);
    }
    
    inline
    float popReal(FnParams* p)
    {
        auto i = LStackTopVarObj(p->stack).lock();
        RealVarObj* vo = dynamic_cast<RealVarObj*>(i.get());
        p->stack->pop();
        return vo ? vo->value() : 0;
    }
    
    inline
    float popReal(LStack* stack)
    {
        auto i = LStackTopVarObj(stack).lock();
        RealVarObj* vo = dynamic_cast<RealVarObj*>(i.get());
        stack->pop();
        return vo ? vo->value() : 0;
    }

    inline
    int popInt(FnParams* p)
    {
        auto i = LStackTopVarObj(p->stack).lock();
        IntVarObj* vo = dynamic_cast<IntVarObj*>(i.get());
        p->stack->pop();
        return vo ? vo->value() : 0;
    }
    
    inline
    int popInt(LStack* stack)
    {
        auto i = LStackTopVarObj(stack).lock();
        IntVarObj* vo = dynamic_cast<IntVarObj*>(i.get());
        stack->pop();
        return vo ? vo->value() : 0;
    }
    
    inline
    void pushString(FnParams* p, const char* s)
    {
        LStackPushVarObj(p->stack, std::move(StringVarObj::createString("string", s)));
    }

}
