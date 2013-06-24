
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "VarObjArray.h"

#include "LandruVM/Fiber.h"
#include "LandruVM/VarObjStackUtil.h"

namespace Landru {

    LANDRU_DECL_FN(VarObjArray, create)
	{
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);
	}
    
    LANDRU_DECL_TABLE_BEGIN(VarObjArray)
    LANDRU_DECL_ENTRY(VarObjArray, create)
    LANDRU_DECL_TABLE_END(VarObjArray)

} // Landru
