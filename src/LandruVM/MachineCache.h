
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include <vector>

#include "LandruVM/VarObj.h"

namespace Landru
{
    
    class Exemplar;
    class Fiber;

    struct MachineCacheEntry
    {
        MachineCacheEntry(std::unique_ptr<Exemplar> e);
        MachineCacheEntry(const MachineCacheEntry& mc);
        
        /// @TODO The program store should go here instead of
        /// on every fiber. In order for that to happen, there
        /// has to be a way to reference vars per fiber, without
        /// modifying the program store per fiber

        // Shared variables are keyed from the exemplar
        std::shared_ptr<Exemplar>	exemplar;
        std::shared_ptr<VarObjArray> sharedVars;
    };

    class MachineCache
    {
    public:
        MachineCache();
        ~MachineCache();        
        std::shared_ptr<MachineCacheEntry> findExemplar(const char* name);
        void add(VarObjFactory* factory, std::shared_ptr<Exemplar> e);        
        
        std::shared_ptr<Fiber> createFiber(VarObjFactory* factory, char const* name);
        
    private:
        std::vector<std::shared_ptr<MachineCacheEntry>> cache;
    };
    
} // Landru

