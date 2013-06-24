
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "RealVarObj.h"

#include "LandruVM/Engine.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/Stack.h"
#include "LandruVM/VarObjStackUtil.h"

namespace Landru {

	RealVarObj::RealVarObj(const char* name)
    : VarObj(name, &functions)
    , v(0)
	{
	}

	RealVarObj::~RealVarObj()
	{
	}
    
    LANDRU_DECL_FN(RealVarObj, add)
    {
        RealVarObj* o = (RealVarObj*) p->vo;
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

        o->v += voa->getReal(-1);
    }
    
    LANDRU_DECL_FN(RealVarObj, set)
    {
        RealVarObj* o = (RealVarObj*) p->vo;
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

        o->v = voa->getReal(-1);
    }

    LANDRU_DECL_TABLE_BEGIN(RealVarObj)
        LANDRU_DECL_ENTRY(RealVarObj, add)
        LANDRU_DECL_ENTRY(RealVarObj, set)
    LANDRU_DECL_TABLE_END(RealVarObj)

}
