
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "IntVarObj.h"

#include "LandruVM/VarObjStackUtil.h"
#include "LandruVM/Engine.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/GeneratorVarObj.h"
#include "LandruVM/Stack.h"

namespace Landru {

    class IntRangeGenerator : public GeneratorVarObj::Generator {
    public:
        IntRangeGenerator(int first, int last, int incr)
        : first(first), last(last), incr(incr), curr(first) {}
        virtual ~IntRangeGenerator() {}

        virtual void begin(VarObjArray* params) {}
        virtual bool done() { return curr > last; }
        virtual void next() { curr += incr; }

        virtual void generate(VarObjArray& locals) {
            std::shared_ptr<VarObj> i = std::make_shared<IntVarObj>("int");
            IntVarObj* ivo = (IntVarObj*) i.get();
            ivo->set(curr <= last ? curr : last);
            locals.push_back(i);
        }

        int first, last, incr, curr;
    };

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

    LANDRU_DECL_FN(IntVarObj, range)
    {
        std::shared_ptr<VarObjArray> voa = p->stack->top<VarObjArray>();
        p->stack->pop();
        int first, last, incr;
        if (voa->size() == 3) {
            first = voa->getInt(-3);
            last = voa->getInt(-2);
            incr = voa->getInt(-1);
        }
        else {
            first = voa->getInt(-2);
            last = voa->getInt(-1);
            incr = last>first ? 1 : -1;
        }
        std::shared_ptr<GeneratorVarObj> i = std::make_shared<GeneratorVarObj>("intGen");
        i->generator = new IntRangeGenerator(first, last, incr);
        p->stack->push(i);
    }

    LANDRU_DECL_TABLE_BEGIN(IntVarObj)
        LANDRU_DECL_ENTRY(IntVarObj, add)
        LANDRU_DECL_ENTRY(IntVarObj, set)
        LANDRU_DECL_ENTRY(IntVarObj, range)
    LANDRU_DECL_TABLE_END(IntVarObj)

}
