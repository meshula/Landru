
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruVM/VarObj.h"
#include "LandruVM/RaiseError.h"

#include "LabText/dict.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

namespace Landru
{

	// VarObj Factory
    
    static unsigned int _dictStringCopyHTHashFunction(const void *key)
    {
        return dictGenHashFunction((const unsigned char*) key, strlen((const char*) key));
    }
    
    static void *_dictStringDup(void *privdata, const void *key)
    {
        int len = strlen((const char*) key);
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
    
    // this pattern allows FactoryRegister to be called during static initialization
    dict* factories()
    {
        static dict* d = dictCreate(&dictTypeStaticStrings, 0);
        return d;
    }
    
	VarObjPtr* VarObj::Factory(VarPool* v, void* key, int index, const char* type, const char* name)
	{
        VarObjPtr* (*fn)(VarPool*, void*, int, const char*) = 
            (VarObjPtr* (*)(VarPool*, void*, int, const char*)) dictFetchValue(factories(), type);
        return fn ? fn(v, key, index, name) : 0;
	}
    
	VarObjPtr* VarObj::Factory(VarPool* v, void* key, int index, void* type, const char* name)
	{
        return Factory(v, key, index, (const char*) type, name);
    }
    
	void VarObj::FactoryRegister(const char* type, VarObjPtr* (*fn)(VarPool*, void*, int, const char*))
	{
		printf("registering %s\n", type);
        dictAdd(factories(), (void*) type, (void*) fn);
	}
	
    dict* strings()
    {
        static dict* d = dictCreate(&dictTypeStaticStrings, 0);
        return d;
    }
    
	// lockCount of -1 means not shared
    VarObj::VarObj(const char* name, FnTable* f)
    : lockCount(-1)
    , _vars(0)
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
        
    int VarObj::GetVarIndex(const char* s) const 
    { 
        VarObjPtr* vop = _vars->varObj(this, s);
        if (vop)
            return vop->i;
        return -1;
    }
    
    VarObjPtr* VarObj::GetVar(int i) const 
    { 
        return _vars->varObj(this, i); 
    }
    
    VarObjPtr* VarObj::GetVar(const char* s) const 
    { 
        return _vars->varObj(this, s); 
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

    VarObjPtr* VarPool::varObj(const void* key, const char* name) const
    {
        if (!name)
            return 0;
        
        try {
            std::lock_guard<std::mutex> lock(poolLock);
            
            for (int i = 0; i < poolSize; ++i) {
                if (pool[i].prevFree != pool[i].nextFree)
                    continue;
                
                if (pool[i].key != key)
                    continue;
                
                if (pool[i].vo->name() && !strcmp(name, pool[i].vo->name()))
                    return &pool[i];
            }
            
        }
        catch(std::exception&e) {
            std::cout << e.what();
        }
        
        return 0;
    }
    
    

} // Landru


void LVarObjIncRefCount(LVarPool* v, LVarObj* lvo)
{
    Landru::VarPool* varpool = (Landru::VarPool*) v;
	Landru::VarObjPtr* vo = (Landru::VarObjPtr*) lvo;
    varpool->addStrongRef(vo);
}

void LVarObjDecRefCount(LVarPool* v, LVarObj* lvo)
{
    Landru::VarPool* varpool = (Landru::VarPool*) v;
	Landru::VarObjPtr* vo = (Landru::VarObjPtr*) lvo;
    varpool->releaseStrongRef(vo);
}


