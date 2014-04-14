
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "RealVarObj.h"

#include "LandruVM/Engine.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/GeneratorVarObj.h"
#include "LandruVM/Stack.h"
#include "LandruVM/VarObjStackUtil.h"

namespace Landru {

    class RealRangeGenerator : public GeneratorVarObj::Generator {
    public:
        RealRangeGenerator(float first, float last, float incr)
        : first(first), last(last), incr(incr), curr(first) {}
        virtual ~RealRangeGenerator() {}

        virtual void begin(VarObjArray* params) {}
        virtual bool done() { return curr > last; }
        virtual void next() { curr += incr; }

        virtual void generate(VarObjArray& locals) {
            std::shared_ptr<VarObj> i = std::make_shared<RealVarObj>("real");
            RealVarObj* ivo = (RealVarObj*) i.get();
            ivo->set(curr <= last ? curr : last);
            locals.push_back(i);
        }

        float first, last, incr, curr;
    };

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
        RealVarObj* o = (RealVarObj*) p->vo.get();
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();

        o->v += voa->getReal(-1);
    }
    
    LANDRU_DECL_FN(RealVarObj, set)
    {
        RealVarObj* o = (RealVarObj*) p->vo.get();
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();
        o->v = voa->getReal(-1);
    }

    LANDRU_DECL_FN(RealVarObj, range)
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();
        float first, last, incr;
        if (voa->size() == 3) {
            first = voa->getReal(-3);
            last = voa->getReal(-2);
            incr = voa->getReal(-1);
        }
        else {
            first = voa->getReal(-2);
            last = voa->getReal(-1);
            incr = last>first ? 1.f : -1.f;
        }
        std::shared_ptr<GeneratorVarObj> i = std::make_shared<GeneratorVarObj>("intGen");
        i->generator = new RealRangeGenerator(first, last, incr);
        p->stack->push(i);
    }

    LANDRU_DECL_TABLE_BEGIN(RealVarObj)
        LANDRU_DECL_ENTRY(RealVarObj, add)
        LANDRU_DECL_ENTRY(RealVarObj, set)
        LANDRU_DECL_ENTRY(RealVarObj, range)
    LANDRU_DECL_TABLE_END(RealVarObj)

}
