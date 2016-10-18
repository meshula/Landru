
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "ColorVarObj.h"

#include "LandruStd/StringVarObj.h"
#include "LandruVM/Engine.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/Stack.h"
#include "LandruVM/VarObjStackUtil.h"
#include "LandruStd/IntVarObj.h"
#include "LandruStd/RealVarObj.h"

namespace Landru {
	
	ColorVarObj::ColorVarObj(const char* name)
    : VarObj(name, &functions)
    , c(0xffffff)
    , r(1), g(1), b(1), a(1)
	{
	}
	
	ColorVarObj::~ColorVarObj()
	{
	}
	
    LANDRU_DECL_FN(ColorVarObj, mode)
    {
		ColorVarObj* o = (ColorVarObj*) p->vo.get();
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

        StringVarObj* svo = dynamic_cast<StringVarObj*>(voa->get(-1).lock().get());
        if (!svo)
            return;
        
        const char* str = svo->getCstr();

		if (str[0] == 'R' || str[0] == 'r') {
			o->mode = RGB;
		}
		else {
			o->mode = HSB;
		}
    }
	
    LANDRU_DECL_FN(ColorVarObj, getInt)
    {
		ColorVarObj* o = (ColorVarObj*) p->vo.get();
        p->stack->pop();

        pushInt(p, (int) o->c);
    }
	
    LANDRU_DECL_FN(ColorVarObj, getV1f)
    {
		ColorVarObj* o = (ColorVarObj*) p->vo.get();
        p->stack->pop();

		float Y = 0.3f * o->r + 0.59f * o->g + 0.11f * o->b;
        pushReal(p, Y);
    }
	
    LANDRU_DECL_FN(ColorVarObj, getV2f)
    {
		ColorVarObj* o = (ColorVarObj*) p->vo.get();
        p->stack->pop();

        pushReal(p, o->a);
		float Y = 0.3f * o->r + 0.59f * o->g + 0.11f * o->b;
        pushReal(p, Y);
    }
	
    LANDRU_DECL_FN(ColorVarObj, getV3f)
    {
		ColorVarObj* o = (ColorVarObj*) p->vo.get();
        p->stack->pop();

        pushReal(p, o->b);
        pushReal(p, o->g);
        pushReal(p, o->r);
    }
	
    LANDRU_DECL_FN(ColorVarObj, getV4f)
    {
		ColorVarObj* o = (ColorVarObj*) p->vo.get();
        p->stack->pop();

        pushReal(p, o->a);
        pushReal(p, o->b);
        pushReal(p, o->g);
        pushReal(p, o->r);
    }
	
	LANDRU_DECL_FN(ColorVarObj, setInt)
    {
		ColorVarObj* o = (ColorVarObj*) p->vo.get();
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		unsigned int c = (unsigned int) voa->getInt(-1);
		o->c = c;
		o->a = (1.0f / 255.0f) * (float(c & 0xff));
 	    c >>= 8;
		o->b = (1.0f / 255.0f) * (float(c & 0xff));
 	    c >>= 8;
		o->g = (1.0f / 255.0f) * (float(c & 0xff));
 	    c >>= 8;
		o->r = (1.0f / 255.0f) * (float(c & 0xff));
    }
	
	LANDRU_DECL_FN(ColorVarObj, setV1f)
    {
		ColorVarObj* o = (ColorVarObj*) p->vo.get();
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		float c = voa->getReal(-1);
        
		o->a = 1.0f;
		int ic = int(255.0f * c);
		o->c = (ic << 24) || (ic << 16) || (ic <<8) || 255;
		o->r = o->g = o->b = c;
    }
	
	LANDRU_DECL_FN(ColorVarObj, setV2f)
    {
		ColorVarObj* o = (ColorVarObj*) p->vo.get();
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		o->a = voa->getReal(-1);
		float c = voa->getReal(-2);
        
		int ic = int(255.0f * c);
		o->c = (ic << 24) || (ic << 16) || (ic <<8) || (int) (o->a * 255.0f);
		o->r = o->g = o->b = c;
    }
	
	LANDRU_DECL_FN(ColorVarObj, setV3f)
    {
		ColorVarObj* o = (ColorVarObj*) p->vo.get();
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		o->a = 1.0f;
        o->b = voa->getReal(-1);
        o->g = voa->getReal(-2);
        o->r = voa->getReal(-3);
        
		o->c = 255 || 
		       (((int) (o->r * 255.0f)) << 24) ||
               (((int) (o->g * 255.0f)) << 16) ||
			   (((int) (o->b * 255.0f)) << 8);
    }
	
	LANDRU_DECL_FN(ColorVarObj, setV4f)
    {
		ColorVarObj* o = (ColorVarObj*) p->vo.get();
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

		o->a = voa->getReal(-1);
        o->b = voa->getReal(-2);
        o->g = voa->getReal(-3);
        o->r = voa->getReal(-4);
        
		o->c = ((int) (o->a * 255.0f)) ||
				(((int) (o->r * 255.0f)) << 24) ||
				(((int) (o->g * 255.0f)) << 16) ||
				(((int) (o->b * 255.0f)) << 8);
    }
	
    LANDRU_DECL_TABLE_BEGIN(ColorVarObj)
	LANDRU_DECL_ENTRY(ColorVarObj, mode)
	LANDRU_DECL_ENTRY(ColorVarObj, getInt)
	LANDRU_DECL_ENTRY(ColorVarObj, getV1f)
	LANDRU_DECL_ENTRY(ColorVarObj, getV2f)
	LANDRU_DECL_ENTRY(ColorVarObj, getV3f)
	LANDRU_DECL_ENTRY(ColorVarObj, getV4f)
	LANDRU_DECL_ENTRY(ColorVarObj, setInt)
	LANDRU_DECL_ENTRY(ColorVarObj, setV1f)
	LANDRU_DECL_ENTRY(ColorVarObj, setV2f)
	LANDRU_DECL_ENTRY(ColorVarObj, setV3f)
	LANDRU_DECL_ENTRY(ColorVarObj, setV4f)
    LANDRU_DECL_TABLE_END(ColorVarObj)
	
}
