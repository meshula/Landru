
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruStd/JsonVarObj.h"

#include "LandruStd/StringVarObj.h"
#include "LandruVM/Engine.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/Stack.h"
#include "LandruVM/VarObjStackUtil.h"

#include "LabJson/json.h"

namespace Landru
{
    
    JsonVarObj::JsonVarObj(const char* name)
    : VarObj(name, &functions)
    , jsonValue(0)
    , owner(false)
    {
    }
    
    JsonVarObj::~JsonVarObj()
    {
        if (owner)
            delete jsonValue;
    }
    
    Json::Value* JsonVarObj::json()
    {
        return jsonValue;
    }
    
    void JsonVarObj::setJson(Json::Value* jv)
    {
        if (owner)
            delete jsonValue;
        
        jsonValue = jv;
        owner = false;
    }
    
    VarObjPtr* JsonVarObj::createJson(VarPool* v, const char* name)
    {
        JsonVarObj* jvo = new JsonVarObj(name);
        return v->allocVarObjSlot(0, jvo, true); // adds strong ref count
    }
    
    LANDRU_DECL_FN(JsonVarObj, copy)
	{
		JsonVarObj* o = (JsonVarObj*) p->vo;
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

        VarObj* var = voa->get(-1)->vo;
        JsonVarObj* copyJson = dynamic_cast<JsonVarObj*>(var);
        if (copyJson) {
            o->jsonValue = new Json::Value(*(copyJson->jsonValue));
            o->owner = true;
        }
	}

    LANDRU_DECL_FN(JsonVarObj, set)
    {
		JsonVarObj* o = (JsonVarObj*) p->vo;
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);

        StringVarObj* strValue = dynamic_cast<StringVarObj*>(voa->get(-1)->vo);
        StringVarObj* strPath = dynamic_cast<StringVarObj*>(voa->get(-2)->vo);
        if (o->jsonValue && strValue && strPath) {
            Json::Path path(strPath->getString());
            Json::Value& v = path.make(*(o->jsonValue));
            if (v.type() == Json::intValue) {
                int i = atoi(strValue->getCstr());
                v = i;
            }
            else if (v.type() == Json::uintValue) {
                unsigned int i = (unsigned int) atoi(strValue->getCstr());
                v = i;
            }
            else if (v.type() == Json::booleanValue) {
                bool i = atoi(strValue->getCstr()) > 0;
                v = i;
            }
            else if (v.type() == Json::realValue) {
                float i = atof(strValue->getCstr());
                v = i;
            }
            else
                v = strValue->getString();
        }
    }
    
    LANDRU_DECL_TABLE_BEGIN(JsonVarObj)
    LANDRU_DECL_ENTRY(JsonVarObj, copy)
    LANDRU_DECL_ENTRY(JsonVarObj, set)
    LANDRU_DECL_TABLE_END(JsonVarObj)
    
}
