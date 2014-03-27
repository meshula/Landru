
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "StringVarObj.h"

#include "LandruVM/Engine.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/Stack.h"
#include "LandruVM/VarObjStackUtil.h"

namespace Landru {

    std::unique_ptr<VarObj> StringVarObj::createString(const char* name)
    {
        return createString(name, "");
    }
    
    std::unique_ptr<VarObj> StringVarObj::createString(const char* name, const char* value)
    {
        return std::unique_ptr<VarObj>(new StringVarObj(name, value));
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
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();
		StringVarObj* o = reinterpret_cast<StringVarObj*>(voa->get(-1).lock().get());
        StringVarObj* cp = reinterpret_cast<StringVarObj*>(voa->get(-2).lock().get());
        if (cp) {
            o->v = cp->v;
        }
    }
    
    LANDRU_DECL_TABLE_BEGIN(StringVarObj)
        LANDRU_DECL_ENTRY(StringVarObj, copy)
    LANDRU_DECL_TABLE_END(StringVarObj)

}
