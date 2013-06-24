
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruVM/MachineCache.h"
#include "LandruVM/Exemplar.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/VarObj.h"

#include <string.h>

#define MAX_STACK_DEPTH 32
#define MAX_VARS 32

namespace Landru
{
    MachineCacheEntry::MachineCacheEntry(Exemplar* e) 
    : exemplar(e)
    { 
    }
    
    MachineCacheEntry::MachineCacheEntry(const MachineCacheEntry& mc) 
    : exemplar(mc.exemplar) 
    { 
    }
    
    MachineCache::MachineCache(VarPool* v)
    : varpool(v)
    {
    }
    
    MachineCache::~MachineCache()
    {
        for (std::vector<MachineCacheEntry>::iterator it = cache.begin(); it != cache.end(); ++it)
            delete (*it).exemplar;
    }
    
    const MachineCacheEntry* MachineCache::findExemplar(const char* name)
    {
        for (std::vector<MachineCacheEntry>::const_iterator i = cache.begin(); i != cache.end(); ++i) {
            if (!strcmp((*i).exemplar->nameStr, name))
                return &(*i);
        }
        return 0;
    }
    
    void MachineCache::add(Exemplar* e1)
    {
        // Supplied exemplar might be used in many different machine contexts,
        // but MachineCache refers to a single context; it's fine to JIT a copy
        // of the exemplar per context, but not across machine contexts -
        // different engines might instantiate different libraries causing
        // different resolutions of function pointers.
        //
        Exemplar* e = new Exemplar();
        e->copy(e1);
        
        for (int i = 0; i < e->sharedVarCount; ++i) {
            const char* varType = &e->stringData[e->stringArray[e->sharedVarTypeIndex[i]]];
            const char* varName = &e->stringData[e->stringArray[e->sharedVarNameIndex[i]]];
            VarObjPtr* v = VarObj::Factory(varpool, e, i, varType, varName);
            if (v == 0)
                RaiseError(0, "Unknown variable type", varType);
            v->vo->Shared(true);
        }
        cache.push_back(MachineCacheEntry(e));
    }   
    
    VarObjPtr* MachineCache::createFiber(char const* name)
    {
        const MachineCacheEntry* mce = findExemplar(name);
        if (mce) {
            static int uniqueId = 0;
            VarObjPtr* vop = VarObj::Factory(varpool, this, ++uniqueId, "fiber", name);
            Fiber* f = (Fiber*) vop->vo;
            f->Create(varpool, MAX_STACK_DEPTH, MAX_VARS, vop, mce);
            return vop;
        }
        return 0;
    }
    


} // Landru
