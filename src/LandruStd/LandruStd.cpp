
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruStd.h"

#include "LandruVM/Fiber.h"
#include "ColorVarObj.h"
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

void init_LandruStd(VarObjFactory* vf)
{
    ColorVarObj::InitVarObj(vf);
    Fiber::InitVarObj(vf);
    FileVarObj::InitVarObj(vf);
    IntVarObj::InitVarObj(vf);
    IoVarObj::InitVarObj(vf);
    JsonVarObj::InitVarObj(vf);
    MathVarObj::InitVarObj(vf);
    MouseVarObj::InitVarObj(vf);
    RealVarObj::InitVarObj(vf);
    StringVarObj::InitVarObj(vf);
    TimeVarObj::InitVarObj(vf);
    VarObjArray::InitVarObj(vf);
}

