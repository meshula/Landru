
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRURUNTIME_STACK_H
#define LANDRURUNTIME_STACK_H

#include <stddef.h>
#include "LandruVM/VarObjArray.h"

#ifndef EXTERNC
	#ifdef __cplusplus
		#define EXTERNC extern "C"
	#else
		#define EXTERNC
	#endif
#endif

typedef struct LVarPool LVarPool;
typedef struct LVarObj LVarObj;
typedef struct LFiber LFiber;

typedef struct LStack
{
    Landru::VarObjArray* voa;
	int lock;
} LStack;

EXTERNC LStack* LStackGetFromPool(int depth);
EXTERNC void    LStackReleaseToPool(LStack*, LVarPool*);

EXTERNC LStack* LStackCreate(int depth);
EXTERNC void LStackFree(LStack* ls);
EXTERNC int LStackDepth(LStack* ls);
EXTERNC void LStackPushVarObj(LStack* ls, LVarPool* v, LVarObj* vo);
EXTERNC void LStackPushParamMark(LStack* ls);
EXTERNC void LStackFinalizeParamMark(LStack* ls, LVarPool* v);
EXTERNC void LStackDup(LStack* ls, LVarPool* v);
EXTERNC void LStackPop(LStack* ls, LVarPool* v);
EXTERNC void LStackPopAll(LStack* ls, LVarPool* v);

EXTERNC int LStackTopStringIndex(LStack* ls, LVarPool* v);
EXTERNC LVarObj* LStackTopFiber(LStack* ls, LVarPool*);
EXTERNC LVarObj* LStackTopVarObj(LStack* ls, LVarPool* v);

EXTERNC LVarObj* LStackVarObjAt(LStack* ls, int index);

#endif
