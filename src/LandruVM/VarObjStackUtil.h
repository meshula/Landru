
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
    
    template <class VarObjType>
    class Pop
    {
    public:
        Pop(FnParams* p, VarObjType*& t)
        : vp((VarObjPtr*) LStackTopVarObj(p->stack, p->engine->lvarPool()))
        {
            /// ASSERT(VarObjType::staticTypeId() == vp.vo->vo->typeId());
            t = dynamic_cast<VarObjType*>(vp.vo->vo);
            LStackPop(p->stack, p->engine->lvarPool());
        }
        
        Pop(LStack* stack, VarPool* varpool, VarObjType*& t)
        : vp((VarObjPtr*) LStackTopVarObj(stack, (LVarPool*) varpool))
        {
            /// ASSERT(VarObjType::staticTypeId() == vp.vo->vo->typeId());
            t = dynamic_cast<VarObjType*>(vp.vo->vo);
            LStackPop(stack, (LVarPool*) varpool);
        }
        
        ~Pop() { }
        
        VarObjStrongRef vp;
    };
    
    
    template <class VarObjType>
    VarObjPtr* allocVarObj(FnParams* p, VarObjType*& vo, const char* name)
    {
        vo = new VarObjType(name);
        return p->engine->varPool()->allocVarObjSlot(0, vo, true);
    }
    
    inline
    void pushVarObj(FnParams* p, VarObjPtr* vop)
    {
        LStackPushVarObj(p->stack, p->engine->lvarPool(), (LVarObj*) vop);
    }

    inline
    void pushInt(FnParams* p, int v)
    {
        IntVarObj* i;
        VarObjPtr* vop = allocVarObj<IntVarObj>(p, i, "int");
        i->set(v);
        pushVarObj(p, vop);
        p->engine->varPool()->releaseStrongRef(vop); // release inc done by alloc
    }
    
    inline
    void pushInt(LStack* s, VarPool* varpool, int v)
    {
        IntVarObj* vo = new IntVarObj("int");
        vo->set(v);
        VarObjPtr* vop = varpool->allocVarObjSlot(0, vo, true);
        LStackPushVarObj(s, (LVarPool*) varpool, (LVarObj*) vop);
        varpool->releaseStrongRef(vop); // release inc done by alloc
    }

    inline
    void pushReal(FnParams* p, float v)
    {
        RealVarObj* i;
        VarObjPtr* vop = allocVarObj<RealVarObj>(p, i, "real");
        i->set(v);
        pushVarObj(p, vop);
        p->engine->varPool()->releaseStrongRef(vop); // release inc done by alloc
    }
    
    inline
    void pushReal(LStack* s, VarPool* varpool, float v)
    {
        RealVarObj* vo = new RealVarObj("real");
        vo->set(v);
        VarObjPtr* vop = varpool->allocVarObjSlot(0, vo, true);
        LStackPushVarObj(s, (LVarPool*) varpool, (LVarObj*) vop);
        varpool->releaseStrongRef(vop); // release inc done by alloc
    }
    
    inline
    float popReal(FnParams* p)
    {
        VarObjStrongRef vp((VarObjPtr*) LStackTopVarObj(p->stack, p->engine->lvarPool()));
        RealVarObj* vo = dynamic_cast<RealVarObj*>(vp.vo->vo);
        LStackPop(p->stack, p->engine->lvarPool());
        return vo->value();
    }
    
    inline
    float popReal(LStack* stack, VarPool* varpool)
    {
        VarObjStrongRef vp((VarObjPtr*) LStackTopVarObj(stack, (LVarPool*) varpool));
        RealVarObj* vo = dynamic_cast<RealVarObj*>(vp.vo->vo);
        LStackPop(stack, (LVarPool*) varpool);
        return vo->value();
    }

    inline
    int popInt(FnParams* p)
    {
        VarObjStrongRef vp((VarObjPtr*) LStackTopVarObj(p->stack, p->engine->lvarPool()));
        IntVarObj* vo = dynamic_cast<IntVarObj*>(vp.vo->vo);
        LStackPop(p->stack, p->engine->lvarPool());
        return vo->value();
    }
    
    inline
    int popInt(LStack* stack, VarPool* varpool)
    {
        VarObjStrongRef vp((VarObjPtr*) LStackTopVarObj(stack, (LVarPool*) varpool));
        IntVarObj* vo = dynamic_cast<IntVarObj*>(vp.vo->vo);
        LStackPop(stack, (LVarPool*) varpool);
        return vo->value();
    }
    
    inline
    void pushString(FnParams* p, const char* s)
    {
        VarObjPtr* svo = StringVarObj::createString(p->engine->varPool(), "string", s);
        LStackPushVarObj(p->stack, p->engine->lvarPool(), (LVarObj*) svo);
        p->engine->varPool()->releaseStrongRef(svo); // createString added a strong ref, so did push
    }

}
