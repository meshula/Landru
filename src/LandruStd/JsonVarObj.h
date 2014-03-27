
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include "LandruVM/VarObj.h"

namespace Json { class Value; }

namespace Landru {
    
	class JsonVarObj : public Landru::VarObj
    {
    public:
        JsonVarObj(const char* name);
        virtual ~JsonVarObj();

        //virtual void Update(float elapsedTime)
		//{
		//}
        
        Json::Value* json();
        void setJson(Json::Value*);
        
        VAROBJ_FACTORY(json, JsonVarObj)
        
        LANDRU_DECL_BINDING_BEGIN
        LANDRU_DECL_BINDING(copy)
        LANDRU_DECL_BINDING(set)
        LANDRU_DECL_BINDING_END
        
        Json::Value* jsonValue;
        bool owner; // if true, jsonValue gets deleted on deletion
    };
    
    
}
