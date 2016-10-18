
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruVM/VarObj.h"
#include "LandruVM/VarObjArray.h"
#include "LandruVM/RaiseError.h"

#include "LabText/dict.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <string>

using namespace std;

namespace Landru
{

	// VarObj Factory
    
    static unsigned int _dictStringCopyHTHashFunction(const void *key)
    {
        return (unsigned int) dictGenHashFunction((const unsigned char*) key, (int) strlen((const char*) key));
    }
    
    static void *_dictStringDup(void *privdata, const void *key)
    {
        size_t len = strlen((const char*) key);
        char *copy = (char*) malloc(len+1);
        DICT_NOTUSED(privdata);
        
        memcpy(copy, key, len);
        copy[len] = '\0';
        return copy;
    }
    
    static int _dictStringCopyHTKeyCompare(void *privdata, const void *key1,
                                           const void *key2)
    {
        DICT_NOTUSED(privdata);
        return strcmp((const char*) key1, (const char*) key2) == 0;
    }
    
    static void _dictStringDestructor(void *privdata, void *key)
    {
        DICT_NOTUSED(privdata);
        free(key);
    }
    
    dictType dictTypeHeapStringCopyKey = {
        _dictStringCopyHTHashFunction, /* hash function */
        _dictStringDup,                /* key dup */
        NULL,                          /* val dup */
        _dictStringCopyHTKeyCompare,   /* key compare */
        _dictStringDestructor,         /* key destructor */
        NULL                           /* val destructor */
    };
    
    /* This is like StringCopy but does not auto-duplicate the key.
     * It's used for intepreter's shared strings. */
    dictType dictTypeStaticStrings = {
        _dictStringCopyHTHashFunction, /* hash function */
        NULL,                          /* key dup */
        NULL,                          /* val dup */
        _dictStringCopyHTKeyCompare,   /* key compare */
        NULL,                          /* key destructor */
        NULL                           /* val destructor */
    };
    
    /* This is like StringCopy but also automatically handle dynamic
     * allocated C strings as values. */
    dictType dictTypeHeapStringCopyKeyValue = {
        _dictStringCopyHTHashFunction, /* hash function */
        _dictStringDup,                /* key dup */
        _dictStringDup,                /* val dup */
        _dictStringCopyHTKeyCompare,   /* key compare */
        _dictStringDestructor,         /* key destructor */
        _dictStringDestructor,         /* val destructor */
    };

    std::unique_ptr<VarObj> VarObjFactory::make(const char* type, const char* name) {
        auto i = _factories.find(std::string(type));
        if (i == _factories.end())
            return std::unique_ptr<VarObj>();
        return i->second(name);
    }
    void VarObjFactory::registerFactory(char const*const type, Fn fn) {
		printf("registering %s\n", type);
        _factories[string(type)] = fn;
    }

    dict* strings()
    {
        static dict* d = dictCreate(&dictTypeStaticStrings, 0);
        return d;
    }
    
	// lockCount of -1 means not shared
    VarObj::VarObj(const char* name, FnTable* f)
    : lockCount(-1)
    , _functions(f)
	{
        const char* s = (const char*) dictFetchValue(strings(), name);
        if (!s) {
            char* newS = (char*) malloc(strlen(name) + 1);
            strcpy(newS, name);
            dictAdd(strings(), (void*) name, (void*) newS);
            s = (const char*) dictFetchValue(strings(), name);
        }
        nameStr = s;
	}

	VarObj::~VarObj()
	{
	}
    
    std::weak_ptr<VarObj> VarObj::GetVar(int i) const
    {
        if (i < _vars->size())
            return _vars->get(i);
        return std::weak_ptr<VarObj>();
    }
    std::weak_ptr<VarObj> VarObj::GetSharedVar(int i) const
    {
        if (i < _sharedVars->size())
            return _sharedVars->get(i);
        return std::weak_ptr<VarObj>();
    }

    std::weak_ptr<VarObj> VarObj::GetVar(const char* s) const
    {
        /// @TODO use a dictionary
        int c = _vars->size();
        for (int i = 0; i < c; ++i) {
            if (!strcmp(_vars->get(i).lock()->name(), s))
                return _vars->get(i);
        }
        return std::weak_ptr<VarObj>();
    }
    std::weak_ptr<VarObj> VarObj::GetSharedVar(const char* s) const
    {
        /// @TODO use a dictionary
        int c = _sharedVars->size();
        for (int i = 0; i < c; ++i) {
            if (!strcmp(_sharedVars->get(i).lock()->name(), s))
                return _sharedVars->get(i);
        }
        return std::weak_ptr<VarObj>();
    }

    int VarObj::Func(const char* funcName) {
        if (!_functions)
            return -1;

        std::string s(funcName);

        if (_instanceFunctions) {
            auto i = _instanceFunctions->functions.find(s);
            if (i != _instanceFunctions->functions.end())
                return i->second;
        }
        
        auto i = _functions->functions.find(s);
        if (i == _functions->functions.end())
            return -1;
        
        return i->second;
    }
    
    const int instanceFunctionBase = 32768;
    
    Landru::VarObj::FuncPtr VarObj::Func(int i) {
        if (i < 0)
            return FuncPtr();
        
        if (i >= instanceFunctionBase) {
            if (_instanceFunctions)
                return (_instanceFunctions->functionPtrs)[i];
            else
                return FuncPtr();
        }
        return (_functions->functionPtrs)[i];
    }
    
    void VarObj::AddFunction(const char* funcName, VarObj::FuncPtr fn) {
        if (_functions) {
            int unique = (int) _functions->functions.size();
            std::string s(funcName);
            (_functions->functions)[s] = unique;
            (_functions->functionPtrs)[unique] = fn;
        }
    }

    void VarObj::AddInstanceFunction(const char* funcName, VarObj::FuncPtr fn) {
        if (!_instanceFunctions)
            _instanceFunctions = std::unique_ptr<FnTable>(new FnTable());

        int unique = instanceFunctionBase + (int) _instanceFunctions->functions.size();
        
        std::string s(funcName);
        (_instanceFunctions->functions)[s] = unique;
        (_instanceFunctions->functionPtrs)[unique] = fn;
    }
    
    

} // Landru

