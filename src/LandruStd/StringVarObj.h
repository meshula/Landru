
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LAB_STRINGVAROBJ_H
#define LAB_STRINGVAROBJ_H

#include "LandruVM/VarObj.h"

#include <string>

namespace Landru
{
	class StringVarObj : public Landru::VarObj
	{
		StringVarObj(const char* name, const char* value);
        StringVarObj(const char* name);
		virtual ~StringVarObj();
        
	public:
        
        static VarObjPtr* createString(VarPool*, const char* name);
        static VarObjPtr* createString(VarPool*, const char* name, const char* value);
        
        const char* getCstr() const { return v.c_str(); }
        const std::string& getString() const { return v; }

        virtual void Update(float /*elapsedTime*/) { }
        VAROBJ_FACTORY(string, StringVarObj)

	private:
        std::string v;

        LANDRU_DECL_BINDING_BEGIN
            LANDRU_DECL_BINDING(copy)
        LANDRU_DECL_BINDING_END
	};


}

#endif
