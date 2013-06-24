
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruVM/Stack.h"

#include "LandruVM/RaiseError.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/VarObj.h"
#include "LandruVM/VarObjArray.h"
#include "LandruStd/IntVarObj.h" // LandruStd shouldn't be included here

#include <stdbool.h> // gcc-ism for _Bool
#if defined(__cplusplus) && !defined(_Bool)
    #define _Bool bool
#endif

#include <stdlib.h>
#include <vector>

#define TYPECHECK(i, t) { \
	if (ls->stack[(i)].type != kStackElType ## t) { \
		RaiseError(0, "bad type", #t); return 0; } }


//EXTERNC void LStackReleaseToPool(LStack*, LVarPool*);

namespace {
    /// @TODO, make a real stack pool for multithreading, and suspension
    std::vector<LStack*>* stackPool = 0;
}

LStack* LStackGetFromPool(int maxDepth)
{
    if (stackPool == 0) {
        stackPool = new std::vector<LStack*>();
    }
    if (stackPool->size() == 0) {
        return LStackCreate(maxDepth);
    }
    else {
        LStack* ret = *(stackPool->rbegin());
        stackPool->pop_back();
        return ret;
    }
}

void LStackReleaseToPool(LStack* stack, LVarPool* v)
{
    LStackPopAll(stack, v);
    stackPool->push_back(stack);
}



static _Bool testWrite(int* addr, int currVal, int newVal)
{
	return __sync_bool_compare_and_swap(addr, currVal, newVal);
}

LStack* LStackCreate(int depth)
{
	LStack* ls = (LStack*) malloc(sizeof(LStack));

    ls->voa = new Landru::VarObjArray("stack");
	ls->lock = 0;
    return ls;
}

void LStackFree(LStack* ls)
{
    delete ls->voa;
	free(ls);
}

void LStackPushCheck(LStack* ls)
{
}

void LStackDepthCheck(LStack* ls, int depth)
{
	if (ls->voa->size() < depth)
		RaiseError(0, "stack underflow", "LStackDepthCheck");
}

void LStackPopCheck(LStack* ls)
{
	if (ls->voa->size() < 1)
		RaiseError(0, "stack underflow", "LStackPopCheck");
}

int LStackDepth(LStack* ls)
{
    return ls->voa->size();
}

void LStackAcquireLock(LStack* ls)
{
	while (!testWrite(&ls->lock, 0, 1)) {
		// empty spin to acquire lock
	}
}

void LStackReleaseLock(LStack* ls)
{
	ls->lock = 0;
}

void LStackPushVarObj(LStack* ls, LVarPool* v, LVarObj* vo)
{
	LStackAcquireLock(ls);
	LStackPushCheck(ls);
    ls->voa->add((Landru::VarObjPtr*) vo);
	LStackReleaseLock(ls);
}

void LStackPushParamMark(LStack* ls)
{
	LStackAcquireLock(ls);
	LStackPushCheck(ls);
    ls->voa->add(0);
	LStackReleaseLock(ls);
}

void LStackFinalizeParamMark(LStack* stack, LVarPool* vars)
{
    Landru::VarPool* varPool = (Landru::VarPool*) vars;
    Landru::VarObjPtr* arrayVop = Landru::VarObjArray::Factory(varPool, 0, "params");
    Landru::VarObjArray* voa = (Landru::VarObjArray*) arrayVop->vo;

    int markIndex = stack->voa->size() - 1;
    while (markIndex >= 0 && stack->voa->get(markIndex) != 0)
        --markIndex;

    if (markIndex < 0)
        RaiseError(0, "unmatched parameter mark", "iParamsEnd");

    for (int i = markIndex + 1; i < stack->voa->size(); ++i)
        voa->add(stack->voa->get(i));

    // now pop the parameters and also the parameter mark that had been pushed earlier
    int c = voa->size() + 1;
    for (int i = 0; i < c; ++i)
        LStackPop(stack, vars);

    // and push the aggregated parameters
    LStackPushVarObj(stack, vars, (LVarObj*) arrayVop);
    varPool->releaseStrongRef(arrayVop); // The Factory added a ref, and the push added a ref, so remove the Factory's ref, leaving one ref
}

void LStackDup(LStack* ls, LVarPool* v)
{
	LStackAcquireLock(ls);
	LStackPushCheck(ls);
	LStackPopCheck(ls);
    LVarObj* lvo = LStackTopVarObj(ls, v);
    LStackPushVarObj(ls, v, lvo);
	LStackReleaseLock(ls);
}

void LStackPop(LStack* ls, LVarPool* v)
{
	LStackAcquireLock(ls);
	LStackPopCheck(ls);
    ls->voa->pop();
	LStackReleaseLock(ls);
}

void LStackPopAll(LStack* ls, LVarPool* v)
{
    ls->voa->clear();
}

LVarObj* LStackVarObjAt(LStack* ls, int index)
{
	LStackAcquireLock(ls);
	int actualIndex = index;
	LStackDepthCheck(ls, actualIndex);
    LVarObj* retval = (LVarObj*) ls->voa->get(index);
	LStackReleaseLock(ls);
	return retval;
}

int LStackTopStringIndex(LStack* ls, LVarPool* varpool)
{
    Landru::VarObjStrongRef vp((Landru::VarObjPtr*) LStackTopVarObj(ls, varpool));
    Landru::IntVarObj* vo = reinterpret_cast<Landru::IntVarObj*>(vp.vo->vo);
    return vo->value();
}

LVarObj* LStackTopVarObj(LStack* ls, LVarPool*)
{
	LStackAcquireLock(ls);
	LStackPopCheck(ls);
	LVarObj* retval = (LVarObj*) ls->voa->get(ls->voa->size() - 1);
	LStackReleaseLock(ls);
	return retval;
}

