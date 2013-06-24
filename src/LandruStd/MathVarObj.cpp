
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruStd/MathVarObj.h"

#include "LabText/ftoa.h"
#include "LandruStd/StringVarObj.h"
#include "LandruVM/Engine.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/Stack.h"
#include "LandruVM/VarObjStackUtil.h"

namespace Landru {

    MathVarObj::MathVarObj(const char* name)
    : VarObj(name, &functions)
    {
    }

	LANDRU_DECL_FN(MathVarObj, v2)
	{
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

        pushReal(p, voa->getReal(-2));
        pushReal(p, voa->getReal(-1));
	}

	LANDRU_DECL_FN(MathVarObj, v3)
	{
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

        pushReal(p, voa->getReal(-3));
        pushReal(p, voa->getReal(-2));
        pushReal(p, voa->getReal(-1));
	}

	LANDRU_DECL_FN(MathVarObj, scalev2)
	{
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

        float scale = voa->getReal(-3);
        float y = voa->getReal(-2) * scale;
        float x = voa->getReal(-1) * scale;
        pushReal(p, y);
        pushReal(p, x);
	}

	LANDRU_DECL_FN(MathVarObj, scalev3)
	{
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

        float scale = voa->getReal(-4);
        float z = voa->getReal(-3) * scale;
        float y = voa->getReal(-2) * scale;
        float x = voa->getReal(-1) * scale;
        pushReal(p, z);
        pushReal(p, y);
        pushReal(p, x);
	}

	LANDRU_DECL_FN(MathVarObj, realToString)
    {
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

        float r = voa->getReal(-1);
        char buff[32];
        ftoa(r, buff);
        pushString(p, buff);
    }

    LANDRU_DECL_TABLE_BEGIN(MathVarObj)
        LANDRU_DECL_ENTRY(MathVarObj, v2)
        LANDRU_DECL_ENTRY(MathVarObj, v3)
        LANDRU_DECL_ENTRY(MathVarObj, scalev2)
        LANDRU_DECL_ENTRY(MathVarObj, scalev3)
        LANDRU_DECL_ENTRY(MathVarObj, realToString)
    LANDRU_DECL_TABLE_END(MathVarObj)
}
