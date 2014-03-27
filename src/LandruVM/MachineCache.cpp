
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
    MachineCacheEntry::MachineCacheEntry(std::unique_ptr<Exemplar> e) 
    : exemplar(std::move(e)) { }
    
    MachineCacheEntry::MachineCacheEntry(const MachineCacheEntry& mc) 
    : exemplar(mc.exemplar) {}
    
    MachineCache::MachineCache() {}
    
    MachineCache::~MachineCache() {}
    
    std::shared_ptr<MachineCacheEntry> MachineCache::findExemplar(const char* name) {
        for (auto i = cache.begin(); i != cache.end(); ++i) {
            if (!strcmp((*i).get()->exemplar->nameStr, name))
                return *i;
        }
        return 0;
    }
    
    void MachineCache::add(VarObjFactory* factory, std::shared_ptr<Exemplar> e1) {
        // Supplied exemplar might be used in many different machine contexts,
        // but MachineCache refers to a single context; it's fine to JIT a copy
        // of the exemplar per context, but not across machine contexts -
        // different engines might instantiate different libraries causing
        // different resolutions of function pointers.
        //
        std::unique_ptr<Exemplar> e(new Exemplar());
        e->copy(e1.get());
        
        for (int i = 0; i < e->sharedVarCount; ++i) {
            const char* varType = &e->stringData[e->stringArray[e->sharedVarTypeIndex[i]]];
            const char* varName = &e->stringData[e->stringArray[e->sharedVarNameIndex[i]]];
            std::unique_ptr<VarObj> v = factory->make(varType, varName);
            if (!v)
                RaiseError(0, "Unknown variable type", varType);
            e->vars.push_back(std::shared_ptr<VarObj>(std::move(v)));
        }
        cache.push_back(std::make_shared<MachineCacheEntry>(std::move(e)));
    }   
    
    std::shared_ptr<Fiber> MachineCache::createFiber(VarObjFactory* factory, char const* name)
    {
        std::shared_ptr<MachineCacheEntry> mce = findExemplar(name);
        if (mce) {
            std::shared_ptr<Fiber> f = std::make_shared<Fiber>("fiber");
            f->Create(factory, MAX_STACK_DEPTH, MAX_VARS, mce);
            return f;
        }
        return 0;
    }
    


} // Landru
