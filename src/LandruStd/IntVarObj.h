
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LAB_INTVAROBJ_H
#define LAB_INTVAROBJ_H

#include "LandruVM/VarObj.h"

namespace Landru
{
	class IntVarObj : public Landru::VarObj
	{
	public:
		IntVarObj(const char* name);
		virtual ~IntVarObj();

        virtual void Update(float /*elapsedTime*/) { }
        VAROBJ_FACTORY(int, IntVarObj)

        static VarObjPtr* createInt(VarPool* v, int value);
        int value() const { return v; }
        void set(int val) { v = val; }
        
	private:
        int v;

        LANDRU_DECL_BINDING_BEGIN
            LANDRU_DECL_BINDING(add)
            LANDRU_DECL_BINDING(set)
        LANDRU_DECL_BINDING_END
	};


}

#endif
