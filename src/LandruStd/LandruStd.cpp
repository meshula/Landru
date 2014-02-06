
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruStd.h"

#include "ColorVarObj.h"
#include "Fiber.h"
#include "FileVarObj.h"
#include "IntVarObj.h"
#include "IoVarObj.h"
#include "JsonVarObj.h"
#include "MathVarObj.h"
#include "RealVarObj.h"
#include "StringVarObj.h"
#include "TimeVarObj.h"
#include "VarObjArray.h"

using namespace Landru;

void init_LandruStd()
{
    ColorVarObj::InitVarObj();
    Fiber::InitVarObj();
    FileVarObj::InitVarObj();
    IntVarObj::InitVarObj();
    IoVarObj::InitVarObj();
    JsonVarObj::InitVarObj();
    MathVarObj::InitVarObj();
    MouseVarObj::InitVarObj();
    RealVarObj::InitVarObj();
    StringVarObj::InitVarObj();
    TimeVarObj::InitVarObj();
    VarObjArray::InitVarObj();
}

