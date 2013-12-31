
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LAB_REALVAROBJ_H
#define LAB_REALVAROBJ_H

#include "LandruVM/VarObj.h"

namespace Landru
{
	class RealVarObj : public Landru::VarObj
	{
	public:
		RealVarObj(const char* name);
		virtual ~RealVarObj();

        virtual void Update(float /*elapsedTime*/) { }
        VAROBJ_FACTORY(real, RealVarObj)

        static VarObjPtr* createReal(VarPool* v, int value);
        float value() const { return v; }
        void set(float val) { v = val; }

	private:
        float v;

        LANDRU_DECL_BINDING_BEGIN
            LANDRU_DECL_BINDING(add)
            LANDRU_DECL_BINDING(set)
        LANDRU_DECL_BINDING_END
	};


}

#endif
