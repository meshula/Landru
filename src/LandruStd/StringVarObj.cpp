
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "StringVarObj.h"

#include "LandruVM/Engine.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/Stack.h"
#include "LandruVM/VarObjStackUtil.h"

namespace Landru {

    VarObjPtr* StringVarObj::createString(VarPool* v, const char* name)
    {
        return createString(v, name, "");
    }
    
    VarObjPtr* StringVarObj::createString(VarPool* v, const char* name, const char* value)
    {
        StringVarObj* svo = new StringVarObj(name, value);
        return v->allocVarObjSlot(0, svo, true);    // add a strong ref
    }
    
	StringVarObj::StringVarObj(const char* name)
    : VarObj(name, &functions)
	{
	}
    
    StringVarObj::StringVarObj(const char* name, const char* s)
    : VarObj(name, &functions)
    {
        v.assign(s);
    }

	StringVarObj::~StringVarObj()
	{
	}

    LANDRU_DECL_FN(StringVarObj, copy)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

		StringVarObj* o = reinterpret_cast<StringVarObj*>(voa->get(-1)->vo);
        StringVarObj* cp = reinterpret_cast<StringVarObj*>(voa->get(-2)->vo);
        if (cp) {
            o->v = cp->v;
        }
    }
    
    LANDRU_DECL_TABLE_BEGIN(StringVarObj)
        LANDRU_DECL_ENTRY(StringVarObj, copy)
    LANDRU_DECL_TABLE_END(StringVarObj)

}
