
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "IntVarObj.h"

#include "LandruVM/VarObjStackUtil.h"
#include "LandruVM/Engine.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/Stack.h"

namespace Landru {

	IntVarObj::IntVarObj(const char* name)
    : VarObj(name, &functions)
    , v(0)
	{
	}

	IntVarObj::~IntVarObj()
	{
	}
    
    LANDRU_DECL_FN(IntVarObj, add)
    {
        IntVarObj* o = (IntVarObj*) p->vo.get();
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

        o->v += voa->getInt(-1);
    }
    
    LANDRU_DECL_FN(IntVarObj, set)
    {
        IntVarObj* o = (IntVarObj*) p->vo.get();
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

        o->v = voa->getInt(-1);
    }

    LANDRU_DECL_TABLE_BEGIN(IntVarObj)
        LANDRU_DECL_ENTRY(IntVarObj, add)
        LANDRU_DECL_ENTRY(IntVarObj, set)
    LANDRU_DECL_TABLE_END(IntVarObj)

}
