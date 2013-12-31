
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include "LandruVM/VarObj.h"

namespace Landru
{
	class MathVarObj : public Landru::VarObj
    {
    public:
        MathVarObj(const char* name);
        virtual ~MathVarObj() { }

		virtual void Update(float elapsedTime)
		{
		}

        VAROBJ_FACTORY(math, MathVarObj)

        LANDRU_DECL_BINDING_BEGIN
            LANDRU_DECL_BINDING(cos)
            LANDRU_DECL_BINDING(sin)
            LANDRU_DECL_BINDING(v2)
            LANDRU_DECL_BINDING(v3)
            LANDRU_DECL_BINDING(scalev2)
            LANDRU_DECL_BINDING(scalev3)
            LANDRU_DECL_BINDING(realToString)
        LANDRU_DECL_BINDING_END
    };

}
